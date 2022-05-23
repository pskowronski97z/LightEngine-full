#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <LEGeometry.h>
#include <d3d11.h>
#include <iostream>


#define M_PI 3.14159265358979323846


template<class T>
LightEngine::Geometry<T>::Geometry(std::shared_ptr<Core> core_ptr, std::vector<T> vertices, D3D11_PRIMITIVE_TOPOLOGY topology, std::string name)
	: CoreUser(core_ptr), vertices_vector_(vertices), primitive_topology_(topology), name_(name) {

	indices_vector_.resize(0);
	context_ptr_ = core_ptr->get_context_ptr();

	stride_ = sizeof(T);
	offset_ = 0;

	D3D11_BUFFER_DESC buffer_descriptor_;
	D3D11_SUBRESOURCE_DATA sr_data;
	
	buffer_descriptor_.ByteWidth = sizeof(T)*vertices_vector_.size();
	buffer_descriptor_.Usage = D3D11_USAGE_DEFAULT;
	buffer_descriptor_.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_descriptor_.CPUAccessFlags = 0;
	buffer_descriptor_.MiscFlags = 0;
	buffer_descriptor_.StructureByteStride = sizeof(T);

	sr_data.pSysMem = vertices_vector_.data();

	call_result_ = core_ptr_->get_device_ptr()->CreateBuffer(&buffer_descriptor_, &sr_data, &vertex_buffer_);

	if(FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Vertex buffer creation failed> ", "LEGeometry.cpp",__LINE__, call_result_);
}

template<class T>
const std::vector<T>& LightEngine::Geometry<T>::get_vertices_vector() const {
	return vertices_vector_;
}

template<class T>
void LightEngine::Geometry<T>::set_indices(std::vector<unsigned int> indices) {

	indices_vector_ = indices;

	D3D11_BUFFER_DESC buffer_descriptor_;
	D3D11_SUBRESOURCE_DATA sr_data;

	buffer_descriptor_.ByteWidth = sizeof(unsigned int) * indices_vector_.size();
	buffer_descriptor_.Usage = D3D11_USAGE_DEFAULT;
	buffer_descriptor_.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_descriptor_.CPUAccessFlags = 0;
	buffer_descriptor_.MiscFlags = 0;
	buffer_descriptor_.StructureByteStride = sizeof(unsigned int);

	sr_data.pSysMem = indices_vector_.data();
	sr_data.SysMemPitch = 0;
	sr_data.SysMemSlicePitch = 0;
	
	call_result_ = core_ptr_->get_device_ptr()->CreateBuffer(&buffer_descriptor_, &sr_data, &index_buffer_);

	if(FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Index buffer creation failed> ", "LEGeometry.cpp",__LINE__, call_result_);
}

template<class T>
inline void LightEngine::Geometry<T>::bind_vertex_buffer() {
	context_ptr_->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(), &stride_, &offset_);
}

template<class T>
inline void LightEngine::Geometry<T>::bind_index_buffer() {
	context_ptr_->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT,0);
}

template<class T>
inline void LightEngine::Geometry<T>::bind_topology() {
	context_ptr_->IASetPrimitiveTopology(primitive_topology_);
}

template<class T>
inline void LightEngine::Geometry<T>::draw(int start_loc) {
	context_ptr_->Draw(vertices_vector_.size(), start_loc);
}

template<class T>
inline void LightEngine::Geometry<T>::draw_indexed(int start_loc) {
	context_ptr_->DrawIndexed(indices_vector_.size(), start_loc,0);
}

template<class T>
DirectX::XMVECTOR LightEngine::Geometry<T>::orthogonalize(DirectX::XMFLOAT3 base, DirectX::XMFLOAT3 v) {

	DirectX::XMVECTOR base_ = DirectX::XMVectorSet(base.x, base.y, base.z, 0.0);
	DirectX::XMVECTOR v_ = DirectX::XMVectorSet(v.x, v.y, v.z, 0.0);
	float scale = DirectX::XMVector3Dot(v_, base_).m128_f32[0] / DirectX::XMVector3Dot(base_, base_).m128_f32[0];
	DirectX::XMVECTOR projection = DirectX::XMVectorScale(base_, scale);
	DirectX::XMVECTOR result = DirectX::XMVectorSubtract(v_, projection);
	result = DirectX::XMVector3Normalize(result);

	return result;
}

template<class T>
DirectX::XMFLOAT3 LightEngine::Geometry<T>::calculate_tris_tangent(Vertex3 v1, Vertex3 v2, Vertex3 v3) {
	DirectX::XMFLOAT3 e1;
	DirectX::XMFLOAT3 e2;

	e1.x = v2.position_.x - v1.position_.x;
	e1.y = v2.position_.y - v1.position_.y;
	e1.z = v2.position_.z - v1.position_.z;

	e2.x = v2.position_.x - v3.position_.x;
	e2.y = v2.position_.y - v3.position_.y;
	e2.z = v2.position_.z - v3.position_.z;

	float du_e1 = v2.texcoords_.x - v1.texcoords_.x;
	float dv_e1 = v2.texcoords_.y - v1.texcoords_.y;
	float du_e2 = v2.texcoords_.x - v3.texcoords_.x;
	float dv_e2 = v2.texcoords_.y - v3.texcoords_.y;
	float scale = 1.0f/(du_e1*dv_e2 - du_e2*dv_e1);
	float tx = dv_e2*e1.x - dv_e1*e2.x;
	float ty = dv_e2*e1.y - dv_e1*e2.y;
	float tz = dv_e2*e1.z - dv_e1*e2.z;

	return DirectX::XMFLOAT3(scale * tx, scale * ty, scale * tz);
}

template<>
std::vector<LightEngine::Geometry<LightEngine::Vertex3>> LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(std::shared_ptr<Core> core_ptr, std::string filename, const float **colors_array, const uint32_t colors_array_size) {

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "";
	int total_objects_count = 0;

	if (!reader.ParseFromFile(filename, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();
	std::vector<Vertex3> readed_vertices;
	std::vector<Geometry<Vertex3>> result;

	readed_vertices.reserve(attrib.vertices.size());
	LightEngine::Vertex3 temp_vertex;

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				temp_vertex.position_ = {vx, vy, vz};

				if(s < colors_array_size) 
					temp_vertex.color_ = { colors_array[s][0], colors_array[s][1], colors_array[s][2], 1.0 };	
				else
					temp_vertex.color_ = { 1.0, 1.0, 1.0, 1.0 };

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					temp_vertex.normal_ = {nx, ny, nz};
				}				

				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					temp_vertex.texcoords_ = {tx, ty, 0.0};
				}

				readed_vertices.push_back(temp_vertex);

				if(!(readed_vertices.size()%3)) {
					
					int end_index = readed_vertices.size() - 1;
					Vertex3 &v1 = readed_vertices.at(end_index - 2);
					Vertex3 &v2 = readed_vertices.at(end_index - 1);
					Vertex3 &v3 = readed_vertices.at(end_index);

					DirectX::XMFLOAT3 tris_tangent_vector = calculate_tris_tangent(v1, v2, v3);

					DirectX::XMVECTOR orthogonal_tangent = orthogonalize(v1.normal_, tris_tangent_vector);
					DirectX::XMVECTOR normal = DirectX::XMVectorSet(v1.normal_.x, v1.normal_.y, v1.normal_.z, 0.0);
					DirectX::XMVECTOR bitangent = DirectX::XMVector3Cross(normal, orthogonal_tangent );
					v1.tangent_ = {orthogonal_tangent.m128_f32[0], orthogonal_tangent.m128_f32[1], orthogonal_tangent.m128_f32[2]};
					v1.bitangent_ = {bitangent.m128_f32[0], bitangent.m128_f32[1], bitangent.m128_f32[2]};
					
					orthogonal_tangent = orthogonalize(v2.normal_, tris_tangent_vector);
					normal = DirectX::XMVectorSet(v2.normal_.x, v2.normal_.y, v2.normal_.z, 0.0);
					bitangent = DirectX::XMVector3Cross(normal, orthogonal_tangent );
					v2.tangent_ = {orthogonal_tangent.m128_f32[0], orthogonal_tangent.m128_f32[1], orthogonal_tangent.m128_f32[2]};
					v2.bitangent_ = {bitangent.m128_f32[0], bitangent.m128_f32[1], bitangent.m128_f32[2]};

					orthogonal_tangent = orthogonalize(v3.normal_, tris_tangent_vector);
					normal = DirectX::XMVectorSet(v3.normal_.x, v3.normal_.y, v3.normal_.z, 0.0);
					bitangent = DirectX::XMVector3Cross(normal, orthogonal_tangent );
					v3.tangent_ = {orthogonal_tangent.m128_f32[0], orthogonal_tangent.m128_f32[1], orthogonal_tangent.m128_f32[2]};
					v3.bitangent_ = {bitangent.m128_f32[0], bitangent.m128_f32[1], bitangent.m128_f32[2]};

					}
			}
			index_offset += fv;
		}

		Geometry<Vertex3> object(core_ptr, readed_vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, shapes[s].name);
		result.push_back(object);
		total_objects_count++;
		std::cout<<"<LOADED> "<<shapes[s].name<<std::endl;
		readed_vertices.clear();
	}
	std::cout<<"<"<<total_objects_count<<" object(s) in total>"<<std::endl;
	return result;
}

LightEngine::LightSource::LightSource(std::shared_ptr<Core> core_ptr, float type)
	: ConstantBuffer(core_ptr) {

	parameters.coordinates[0] = 0.0;
	parameters.coordinates[1] = 0.0;
	parameters.coordinates[2] = -0.8;
	parameters.coordinates[3] = type;

	parameters.additional[0] = 1.0;
	parameters.additional[1] = 1.0;
	parameters.additional[2] = 1.0;
	parameters.additional[3] = 1.0;

	parameters.color[0] = 1.0;
	parameters.color[1] = 1.0;
	parameters.color[2] = 1.0;
	parameters.color[3] = 1.0;
	
	try{
		create_constant_buffer<Parameters>(std::make_shared<Parameters>(parameters));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Light source creation failed> ", "LEGeometry.cpp", __LINE__ - 5, call_result_);
	}
}

void LightEngine::LightSource::set_position(float position[3]) {
	parameters.coordinates[0] = position[0];
	parameters.coordinates[1] = position[1];
	parameters.coordinates[2] = position[2];
}

void LightEngine::LightSource::set_color(float color[3]) {
	parameters.color[0] = color[0];
	parameters.color[1] = color[1];
	parameters.color[2] = color[2];
}

void LightEngine::LightSource::update() {
	try{
		update_constant_buffer<Parameters>(std::make_shared<Parameters>(parameters));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Light source (constant buffer) update failed> ", "LEGeometry.cpp", __LINE__ - 5, call_result_);
	}
}

void LightEngine::LightSource::set_intensity(float intensity) {
	parameters.additional[0] = intensity;
}

std::vector<int32_t> LightEngine::GeometryTools::get_k_visible_neighbours(
	const uint32_t k,
	const float r,
	const Vertex3& v0,
	const Vertex3& v1,
	const Vertex3& v2,
	const std::vector<Vertex3>& tested_triangles) {

	std::vector<int32_t> result(k, -1);

	DirectX::XMVECTOR triangle_center_pos = DirectX::XMVectorBaryCentric(
		DirectX::XMVectorSet(v0.position_.x, v0.position_.y, v0.position_.z, 0.0),
		DirectX::XMVectorSet(v1.position_.x, v1.position_.y, v1.position_.z, 0.0),
		DirectX::XMVectorSet(v2.position_.x, v2.position_.y, v2.position_.z, 0.0),
		0.33f,
		0.33f
	);

	DirectX::XMVECTOR triangle_center_n = DirectX::XMVector3Normalize(
		DirectX::XMVectorBaryCentric(
			DirectX::XMVectorSet(v0.normal_.x, v0.normal_.y, v0.normal_.z, 0.0),
			DirectX::XMVectorSet(v1.normal_.x, v1.normal_.y, v1.normal_.z, 0.0),
			DirectX::XMVectorSet(v2.normal_.x, v2.normal_.y, v2.normal_.z, 0.0),
			0.33f,
			0.33f)
	);
	
	for (uint32_t i = 0, triangle_index = 0, itr = 0; i < tested_triangles.size(); i += 3, triangle_index++) {

		DirectX::XMVECTOR sample_triangle_center_pos = DirectX::XMVectorBaryCentric(
			DirectX::XMVectorSet(tested_triangles[i].position_.x, tested_triangles[i].position_.y, tested_triangles[i].position_.z, 0.0),
			DirectX::XMVectorSet(tested_triangles[i + 1].position_.x, tested_triangles[i + 1].position_.y, tested_triangles[i + 1].position_.z, 0.0),
			DirectX::XMVectorSet(tested_triangles[i + 2].position_.x, tested_triangles[i + 2].position_.y, tested_triangles[i + 2].position_.z, 0.0),
			0.33f,
			0.33f
		);

		DirectX::XMVECTOR sample_triangle_center_n = DirectX::XMVector3Normalize(
			DirectX::XMVectorBaryCentric(
				DirectX::XMVectorSet(tested_triangles[i].normal_.x, tested_triangles[i].normal_.y, tested_triangles[i].normal_.z, 0.0),
				DirectX::XMVectorSet(tested_triangles[i + 1].normal_.x, tested_triangles[i + 1].normal_.y, tested_triangles[i + 1].normal_.z, 0.0),
				DirectX::XMVectorSet(tested_triangles[i + 2].normal_.x, tested_triangles[i + 2].normal_.y, tested_triangles[i + 2].normal_.z, 0.0),
				0.33f,
				0.33f
			)
		);

		DirectX::XMVECTOR sight_line = DirectX::XMVectorSubtract(sample_triangle_center_pos, triangle_center_pos);

		float distance = DirectX::XMVector3Length(sight_line).m128_f32[0];

		if ((distance >= r) || (distance < 0.0001f))
			continue;
		
		sight_line = DirectX::XMVector3Normalize(sight_line);

		if (DirectX::XMVector3Dot(sight_line, triangle_center_n).m128_f32[0] <= 0.0f)
			continue;

		sight_line = DirectX::XMVectorScale(sight_line, -1.0f);

		if (DirectX::XMVector3Dot(sight_line, sample_triangle_center_n).m128_f32[0] <= 0.0f)
			continue;

		result.at(itr) = triangle_index;
		itr++;

		if (itr == k)
			break;
	}

	return result;
}

std::vector<std::vector<int32_t>> LightEngine::GeometryTools::get_k_visible_neighbours(
	const uint32_t k, 
	const float r, 
	const std::vector<Vertex3> &testing_triangles, 
	const std::vector<Vertex3> &tested_triangles) {
	
	std::vector<std::vector<int32_t>> result(testing_triangles.size()/3);

	for (int i = 0, j = 0; i < testing_triangles.size(); i += 3, j++) {
		result.at(j) = get_k_visible_neighbours(
			k,
			r,
			testing_triangles.at(i),
			testing_triangles.at(i + 1),
			testing_triangles.at(i + 2),
			tested_triangles
		);
	}
	return result;
}

int32_t *LightEngine::GeometryTools::generate_lookup_matrix(
	const uint32_t objects_count, 
	const uint32_t max_triangles_count, 
	const uint32_t k, 
	const float r, 
	const std::vector<Geometry<Vertex3>>& separated_objects,
	const std::vector<Vertex3>& merged_objects_triangles) {

	const uint32_t depth_pitch = objects_count * max_triangles_count;
	int32_t *result = new int32_t[ depth_pitch * k];
	ZeroMemory(result, depth_pitch * k);
	
	for (int object_index = 0; object_index < separated_objects.size(); object_index++) {

		std::vector<std::vector<int32_t>> object_lookup = get_k_visible_neighbours(
			k,
			r,
			separated_objects.at(object_index).get_vertices_vector(),
			merged_objects_triangles
		);

		for (int triangle_index = 0; triangle_index < object_lookup.size(); triangle_index++) {
			
			for (int neighbour_index = 0; neighbour_index < k; neighbour_index++) {

				uint32_t lookup_dst_index = triangle_index + object_index * max_triangles_count + neighbour_index * depth_pitch;
				result[lookup_dst_index] = object_lookup.at(triangle_index).at(neighbour_index);

			}			
		}
	}
	return result;
}
