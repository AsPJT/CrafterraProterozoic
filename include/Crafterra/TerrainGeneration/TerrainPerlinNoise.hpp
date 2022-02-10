﻿/*##########################################################################################

	Crafterra Library 🌏

	[Planning and Production]
	2017-2022 Kasugaccho
	2018-2022 As Project

	[Contact Us]
	wanotaitei@gmail.com
	https://github.com/AsPJT/Crafterra

	[License]
	Distributed under the CC0 1.0.
	https://creativecommons.org/publicdomain/zero/1.0/

##########################################################################################*/

#ifndef INCLUDED_CRAFTERRA_LIBRARY_CRAFTERRA_TERRAIN_GENERATION_PERLIN_NOISE_ON_FIELD_MAP_HPP
#define INCLUDED_CRAFTERRA_LIBRARY_CRAFTERRA_TERRAIN_GENERATION_PERLIN_NOISE_ON_FIELD_MAP_HPP

#include <Crafterra/Terrain/TerrainInformation.hpp>
#include <Crafterra/TerrainGeneration/HypsographicCurve.hpp>
#include <AsLib2/DataType/PrimitiveDataType.hpp> // int
#include <AsLib2/DataType/IndexArea.hpp>
#include <algorithm>
#include <random>



namespace Crafterra {

	constexpr double getElevationOfSeaLevel(){
		return 110.0;
	}
	// パーリンノイズをフィールドマップ上に生成
	template<typename Matrix_, typename ElevationUint_>
	void generatePerlinNoiseOnFieldMap(
		const Matrix_& matrix_,
		const ::As::IndexUint chunk_index_x_, const ::As::IndexUint chunk_index_y_, const ::As::IndexUint one_chunk_width_, const ::As::IndexUint one_chunk_height_,
		const ::As::IndexAreaXZ& area, ::Crafterra::PerlinNoise& perlin, const double frequency_, const ::As::IndexUint octaves_,
		const bool for_elevation_, const ElevationUint_ max_height_, const ElevationUint_ min_height_ = 0) {

		const ::As::IndexUint end_x_ = area.start_x + area.width;
		const ::As::IndexUint end_y_ = area.start_z + area.depth;
		for (::As::IndexUint row_index{ area.start_z }, row{}; row_index < end_y_; ++row_index, ++row){
			for (::As::IndexUint col_index{ area.start_x }, col{}; col_index < end_x_; ++col_index, ++col){

				double noise = perlin.octaveNoise(octaves_,
								(::As::Uint64(chunk_index_x_) * ::As::Uint64(one_chunk_width_)  + ::As::Uint64(col)) / frequency_,
								(::As::Uint64(chunk_index_y_) * ::As::Uint64(one_chunk_height_) + ::As::Uint64(row)) / frequency_
							);
				if(for_elevation_){
					// ノイズ値、最低/最高標高、険しさ値(0.0 - 1.0)、海面の標高
					noise = Crafterra::processNoiseUsingHypsographicCurve(noise, min_height_, max_height_, 0.85, ::Crafterra::getElevationOfSeaLevel());
				}
				matrix_(col_index, row_index,
					min_height_ + static_cast<ElevationUint_>((max_height_ - min_height_) * noise));
			}
		}
	}

	struct TerrainPerlinNoiseSeed {
		::As::Uint32 temperature;				// 気温
		::As::Uint32 amount_of_rainfall;		// 降水量
		::As::Uint32 elevation;				// 標高
		::As::Uint32 flower;					// 花
		::As::Uint32 lake;						// 湖

		TerrainPerlinNoiseSeed() = default;
		
		template<typename Seed_Gen>
		TerrainPerlinNoiseSeed(Seed_Gen& seed_gen) {
			const ::As::Uint32 temperature_seed = seed_gen();
			const ::As::Uint32 amount_of_rainfall_seed = seed_gen();
			const ::As::Uint32 elevation_seed = seed_gen();
			const ::As::Uint32 flower_seed = seed_gen();
			const ::As::Uint32 lake_seed = seed_gen();
		}

	};

	class TerrainPerlinNoise {
		// 暫定的なマップデータ
		using MapMat = ::As::UniquePtrMatrix<::Crafterra::TerrainInformation>;
		using shape_t = ElevationUint;

		TerrainPerlinNoiseSeed seed;

		::Crafterra::PerlinNoise perlin_temperature_seed;
		::Crafterra::PerlinNoise perlin_amount_of_rainfall_seed;
		::Crafterra::PerlinNoise perlin_elevation_seed;
		::Crafterra::PerlinNoise perlin_flower_seed;
		::Crafterra::PerlinNoise perlin_lake_seed;

	public:
		// コンストラクタ
		TerrainPerlinNoise(const TerrainPerlinNoiseSeed& seed_)
			:
			seed(seed_)
			, perlin_temperature_seed(
				[&seed_](std::array<::As::Uint8, 512>::iterator begin_, std::array<::As::Uint8, 512>::iterator end_) {
					::std::shuffle(begin_, end_, ::std::default_random_engine(seed_.temperature)); })
			, perlin_amount_of_rainfall_seed(
				[&seed_](std::array<::As::Uint8, 512>::iterator begin_, std::array<::As::Uint8, 512>::iterator end_) {
					::std::shuffle(begin_, end_, ::std::default_random_engine(seed_.amount_of_rainfall)); })
						, perlin_elevation_seed(
							[&seed_](std::array<::As::Uint8, 512>::iterator begin_, std::array<::As::Uint8, 512>::iterator end_) {
								::std::shuffle(begin_, end_, ::std::default_random_engine(seed_.elevation)); })
						, perlin_flower_seed(
							[&seed_](std::array<::As::Uint8, 512>::iterator begin_, std::array<::As::Uint8, 512>::iterator end_) {
								::std::shuffle(begin_, end_, ::std::default_random_engine(seed_.flower)); })
									, perlin_lake_seed(
										[&seed_](std::array<::As::Uint8, 512>::iterator begin_, std::array<::As::Uint8, 512>::iterator end_) {
											::std::shuffle(begin_, end_, ::std::default_random_engine(seed_.lake)); })
		{}

	public:

		::As::Uint32 getElevationSeed() const { return this->seed.elevation; }

		void generation(MapMat& terrain_information_matrix, const ::As::IndexUint chunk_index_x_, const ::As::IndexUint chunk_index_y_, const ::As::IndexAreaXZ& area) {
			//温度
			generatePerlinNoiseOnFieldMap(
				[&terrain_information_matrix](const As::IndexUint x_, const As::IndexUint y_, const ElevationUint value_) { terrain_information_matrix[y_][x_].setTemperature(value_); },
				chunk_index_x_, chunk_index_y_, terrain_information_matrix.getWidth() / 2, terrain_information_matrix.getDepth() / 2,
				area,
				perlin_temperature_seed, 40.1, 8,
				false,
				240, 0
			);

			//降水量
			generatePerlinNoiseOnFieldMap(
				[&terrain_information_matrix](const As::IndexUint x_, const As::IndexUint y_, const ElevationUint value_) { terrain_information_matrix[y_][x_].setAmountOfRainfall(value_); },
				chunk_index_x_, chunk_index_y_, terrain_information_matrix.getWidth() / 2, terrain_information_matrix.getDepth() / 2,
				area,
				perlin_amount_of_rainfall_seed, 40.1, 8,
				false,
				240, 0
			);

			//標高
			generatePerlinNoiseOnFieldMap(
				[&terrain_information_matrix](const As::IndexUint x_, const As::IndexUint y_, const ElevationUint value_) { terrain_information_matrix[y_][x_].setElevation(value_); },
				chunk_index_x_, chunk_index_y_, terrain_information_matrix.getWidth() / 2, terrain_information_matrix.getDepth() / 2,
				area,
				perlin_elevation_seed, 600.1, 10,
				true,
				240, 0
			);

			//花
			generatePerlinNoiseOnFieldMap(
				[&terrain_information_matrix](const As::IndexUint x_, const As::IndexUint y_, const double value_) { terrain_information_matrix[y_][x_].setFlower(value_); },
				chunk_index_x_, chunk_index_y_, terrain_information_matrix.getWidth() / 2, terrain_information_matrix.getDepth() / 2,
				area,
				perlin_flower_seed, 1.12345, 1,
				false,
				1.0, 0.0
			);

			//湖
			generatePerlinNoiseOnFieldMap(
				[&terrain_information_matrix](const As::IndexUint x_, const As::IndexUint y_, const ElevationUint value_) { terrain_information_matrix[y_][x_].setLake(value_); },
				chunk_index_x_, chunk_index_y_, terrain_information_matrix.getWidth() / 2, terrain_information_matrix.getDepth() / 2,
				area,
				perlin_lake_seed, 10.12345, 3,
				false,
				200, 50
			);
		}
	};

}

#endif //Included Crafterra Library