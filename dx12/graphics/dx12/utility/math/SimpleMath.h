#pragma once

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <functional>

#undef min
#undef max

namespace hinode
{
	/// @brief 簡単なベクトルや行列演算などまとめたもの
	///
	///	設計方針は以下の通り
	///		・GPUのバッファにそのまま設定できるデータ構造
	///		・シェーダ言語(HLSLがメイン)と同じ文法になるようする(スェルズはまだ)
	///		・処理速度は重視しない(速さが必要な時は他の数学ライブラリを利用する)
	/// 左手系を採用している
	namespace math
	{
		//using uint = uint32_t;

		//  定数宣言
		const float PI = 3.1415926535897932384626433832795f;					///< 円周率
		const float PI_DIV_4 = 0.78539816339744830961566084581988f;			///< Pi / 4	45度
		const float PI_DIV_3 = 1.0471975511965977461542144610932f;			///< Pi / 3	60度
		const float PI_DIV_2 = 1.5707963267948966192313216916398f;			///< Pi / 2	90度
		const float _2_MUL_PI = 6.283185307179586476925286766559f;			///< 2 * Pi	360度
		const float _3_MUL_PI_DIV_2 = 4.7123889803846898576939650749193f;	///< 3 * Pi / 2 270度

		const float DtoR = PI / 180.f;				///< 度からラジアンへ変化するための変数	Pi / 180.f
		const float RtoD = 180.f / PI;				///< ラジアンから度へ変化するための変数	180.f / Pi

		/// @brief floatの同値判定
		/// @param[in] float a
		/// @param[in] float b
		/// @param[in] float epsilon 許容誤差範囲
		/// @retval bool true=同じ, false=異なる
		inline bool nearlyEqual(float a, float b, float epsilon = FLT_EPSILON)noexcept
		{
			return (abs(a - b) <= epsilon);
		}

		/// @brief doubleの同値判定
		/// @param[in] double a
		/// @param[in] double b
		/// @param[in] double epsilon 許容誤差範囲
		/// @retval bool true=同じ, false=異なる
		inline bool nearlyEqual(double a, double b, double epsilon = DBL_EPSILON)noexcept
		{
			return (abs(a - b) <= epsilon);
		}

		/// @brief 近似比較関数
		/// math::bufferで近似同値判定を行うために作成
		/// @tparam T 型
		/// @param[in] T a
		/// @param[in] T b
		/// @param[in] float epsilon 同値許容範囲
		/// @retval bool true=同じ, false=異なる
		template<typename T> inline bool nearlyEqual(T a, T b, typename T::type epsilon)noexcept
		{
			return a.buf.nearlyEqual(b, epsilon);
		}

		/// @brief 比較関数
		/// math::bufferで同値判定を行うために作成
		/// @tparam T 型
		/// @param[in] T a
		/// @param[in] T b
		/// @retval bool true=同じ, false=異なる
		template<typename T> inline bool equal(T a, T b)noexcept
		{
			return a == b;
		}

		/// @brief float型で特殊化した比較関数
		/// math::bufferで同値判定を行うために作成
		/// @param[in] float a
		/// @param[in] float b
		/// @retval bool true=同じ, false=異なる
		template<> inline bool equal(float a, float b)noexcept
		{
			return nearlyEqual(a, b);
		}

		/// @brief double型で特殊化した比較関数
		/// math::bufferで同値判定を行うために作成
		/// @param[in] double a
		/// @param[in] double b
		/// @retval bool true=同じ, false=異なる
		template<> inline bool equal(double a, double b)noexcept
		{
			return nearlyEqual(a, b);
		}

		/// @brief 簡易的な乱数生成
		/// @param[in] min 最小値
		/// @param[in] max 最大値
		/// @retval float 乱数
		inline float simpleRandom(float min, float max)noexcept
		{
			return rand() / (float)RAND_MAX * (max - min) + min;
		}

		//角度関連

		/// @brief 度をラジアンに変換する
		/// @param[in] degree 度表記の角度
		/// @retval float ラジアン表記の角
		inline float toRadian(float degree)noexcept { return degree*DtoR; }
		/// @brief ラジアンを度に変換する
		/// @param[in] degree ラジアン表記の角度
		/// @retval float 度表記の角
		inline float toDegree(float radian)noexcept { return radian*RtoD; }

		/// @brief 角度を0から2PIの範囲に正規化する
		/// @param[in] radian ラジアン
		/// @retval 正規化したラジアン
		inline float normalizeRadian(float radian)noexcept
		{
			while (radian >  PI) radian -= PI * 2;
			while (radian < -PI) radian += PI * 2;
			return radian;
		}

		enum {
			eX, eY, eZ, eW,
		};

		namespace meta
		{
			/// @brief BufferTypeがこちらが想定しているものになっているかチェックする構造体
			template<typename BufferType, typename T = void>
			struct _validate_buffer_type {
				static const bool value = false;
			};

			/// @brief BufferTypeがこちらが想定しているものになっているかチェックする構造体
			template<typename BufferType>
			struct _validate_buffer_type<BufferType, typename std::enable_if<std::is_same<const size_t, decltype(BufferType::ROW)>::value && std::is_same<const size_t, decltype(BufferType::COL)>::value>::type> {
				static const bool value = true;
			};

			template<typename BufferType> struct _is_has_buffer {
				static const bool value = std::conditional<std::is_arithmetic<BufferType>::value, std::false_type, _validate_buffer_type<BufferType>>::type::value;
			};

			template<typename U, typename R>
			struct _compare_min_row {
				static size_t func(const U& a, const R& b)
				{
					using namespace std;
					return min(a.ROW, b.ROW);
				}
			};
			template<typename U, typename R>
			struct _get_row {
				static size_t func(const U& a, const R& b)
				{
					return a.ROW;
				}
			};
			template<typename U, typename R>
			struct _compare_min_col {
				static size_t func(const U& a, const R& b)
				{
					using namespace std;
					return min(a.COL, b.COL);
				}
			};
			template<typename U, typename R>
			struct _get_col {
				static size_t func(const U& a, const R& b)
				{
					return a.COL;
				}
			};

			template<typename U>
			struct _get_buffer_value {
				static typename U::type& func(U& has_buffer, size_t r, size_t c)
				{
					return has_buffer.buf.get(r, c);
				}
				static typename const U::type& func(const U& has_buffer, size_t r, size_t c)
				{
					return has_buffer.buf.get(r, c);
				}
			};

			template<typename U>
			struct _get_value {
				static U& func(U& value, size_t r, size_t c)
				{
					return value;
				}
				static const U& func(const U& value, size_t r, size_t c)
				{
					return value;
				}
			};

			template<bool _is_had_buffer, typename U>
			struct _buffer_element_type;

			template<typename U>
			struct _buffer_element_type<true, U> {
				static_assert(0 < U::ROW && 0 < U::COL, "Don't math::buffer type...");
				typedef typename U::type type;
			};

			template<typename U>
			struct _buffer_element_type<false, U> {
				typedef U type;
			};
		}

		/// @brief 各種の数学関数の処理で実際に使う構造体
		template<typename T, size_t _row, size_t _col>
		struct buffer
		{
			typedef buffer<T, _row, _col> self_type;
			typedef T type;
			static const size_t ROW = _row;
			static const size_t COL = _col;

			/// @brief 指定した場所の値を取得する
			///
			/// @tparam R 行番号(0がはじめ)
			/// @tparam C 列番号(0がはじめ)
			/// @retval T 指定された値
			template<size_t R, size_t C>
			T& get()noexcept
			{
				static_assert(R < _row, "math::buffer::get() : invalid row index!");
				static_assert(C < _col, "math::buffer::get() : invalid col index!");
				return this->m[R][C];
			}

			/// @brief 指定した場所の値を取得する
			///
			/// @tparam R 行番号(0がはじめ)
			/// @tparam C 列番号(0がはじめ)
			/// @retval T 指定された値
			template<size_t R, size_t C>
			const T& get()const noexcept
			{
				static_assert(R < _row, "math::buffer::get() : invalid row index!");
				static_assert(C < _col, "math::buffer::get() : invalid col index!");
				return this->m[R][C];
			}

			/// @brief 指定した場所の値を取得する
			///
			///	コンパイル時にすでに引数がわかる場合はテンプレート版のget()を使用するのを推奨します
			/// @param[in] R 行番号(0がはじめ)
			/// @param[in] C 列番号(0がはじめ)
			/// @retval T 指定された値
			T& get(size_t R, size_t C)noexcept
			{
				assert(R < _row && "math::buffer::get() : invalid row index!");
				assert(C < _col && "math::buffer::get() : invalid col index!");
				return this->m[R][C];
			}

			/// @brief 指定した場所の値を取得する
			///
			///	コンパイル時にすでに引数がわかる場合はテンプレート版のget()を使用するのを推奨します
			/// @param[in] R 行番号(0がはじめ)
			/// @param[in] C 列番号(0がはじめ)
			/// @retval T 指定された値
			const T& get(size_t R, size_t C)const noexcept
			{
				assert(R < _row && "math::buffer::get() : invalid row index!");
				assert(C < _col && "math::buffer::get() : invalid col index!");
				return this->m[R][C];
			}

			/// @brief 指定した場所の値を取得する
			///
			/// @tparam R 行番号(0がはじめ)
			/// @tparam C 列番号(0がはじめ)
			/// @retval T 指定された値
			template<size_t R>
			T* getRow()noexcept
			{
				static_assert(R < _row, "math::buffer::getRow() : invalid row index!");
				return this->m[R];
			}

			/// @brief 指定した場所の値を取得する
			///
			/// @tparam R 行番号(0がはじめ)
			/// @tparam C 列番号(0がはじめ)
			/// @retval T 指定された値
			template<size_t R>
			const T* getRow()noexcept
			{
				static_assert(R < _row, "math::buffer::getRow() : invalid row index!");
				return this->m[R];
			}

			/// @brief 指定した行の先頭ポインタを取得する
			///
			///	コンパイル時にすでに引数がわかる場合はテンプレート版のgetRow()を使用するのを推奨します
			/// @param[in] R 行番号(0がはじめ)
			/// @retval T* 指定された値
			T* getRow(size_t R)noexcept
			{
				assert(R < _row && "math::buffer::getRow() : invalid row index!");
				return this->m[R];
			}

			/// @brief 指定した行の先頭ポインタを取得する
			///
			///	コンパイル時にすでに引数がわかる場合はテンプレート版のgetRow()を使用するのを推奨します
			/// @param[in] R 行番号(0がはじめ)
			/// @retval T* 指定された値
			const T* getRow(size_t R)const noexcept
			{
				assert(R < _row && "math::buffer::getRow() : invalid row index!");
				return this->m[R];
			}

			/// @brief 全要素にアクセスする
			/// @param[in] predicate void(self_type&, size_t row, size_t col)
			void each(std::function<void(const self_type&, size_t row, size_t col)> predicate)const
			{
				for (size_t r = 0; r < this->ROW; ++r) {
					for (size_t c = 0; c < this->COL; ++c) {
						predicate(*this, r, c);
					}
				}
			}

			/// @brief 全要素の値を指定した述語で更新する
			/// @param[in] predicate T(self_type&, size_t row, size_t col)
			void map(std::function<T(self_type&, size_t row, size_t col)> predicate)
			{
				for (size_t r = 0; r < this->ROW; ++r) {
					for (size_t c = 0; c < this->COL; ++c) {
						this->get(r, c) = predicate(*this, r, c);
					}
				}
			}

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator=(const U& right)noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) = static_cast<type>(getter::func(right, r, c));
					}
				}
				return *this;
			}

			/// @brief 単項マイナス演算子のオーバーロード
			/// @retval self_type
			self_type operator-()const noexcept
			{
				self_type result;
				for (size_t r = 0; r < this->ROW; ++r) {
					for (size_t c = 0; c < this->COL; ++c) {
						result.get(r, c) = -1 * this->get(r, c);
					}
				}
				return result;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type operator+(const U& right)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				self_type result;
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						result.get(r, c) = this->get(r, c) + getter::func(right, r, c);
					}
				}
				return result;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator+=(const U& right)noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) += getter::func(right, r, c);
					}
				}
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type operator-(const U& right)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				self_type r;
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						result.get(r, c) = this->get(r, c) - getter::func(right, r, c);
					}
				}
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator-=(const U& right)noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) -= getter::func(right, r, c);
					}
				}
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type operator*(const U& right)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				self_type r;
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						result.get(r, c) = this->get(r, c) * getter::func(right, r, c);
					}
				}
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator*=(const U& right)noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) *= getter::func(right, r, c);
					}
				}
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type operator/(const U& right)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				self_type r;
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						result.get(r, c) = this->get(r, c) / getter::func(right, r, c);
					}
				}
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator/=(const U& right)noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) /= getter::func(right, r, c);
					}
				}
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type operator%(const U& right)const noexcept
			{
				static_assert(std::is_integral<T>::value, "ERROR math::buffer::operator% : T isn't integral type...");
				static_assert(std::is_integral<meta::_buffer_element_type<meta::_is_has_buffer<U>::value, U>::type>::value, "ERROR math::buffer::operator%= : U isn't integral type...");
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				self_type r;
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						result.get(r, c) = this->get(r, c) % getter::func(right, r, c);
					}
				}
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U>
			self_type& operator%=(const U& right)noexcept
			{
				static_assert(std::is_integral<T>::value, "ERROR math::buffer::operator% : T isn't integral type...");
				static_assert(std::is_integral<meta::_buffer_element_type<meta::_is_has_buffer<U>::value, U>::type>::value, "ERROR math::buffer::operator%= : U isn't integral type...");
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				for (size_t r = 0; r < ROW_LOOP_COUNT; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						this->get(r, c) %= getter::func(right, r, c);
					}
				}
				return *this;
			}

			/// @brief 同値判定
			///
			/// 同値判定で使用するかたはthisの型
			/// 判定を行う範囲は( min(this->ROW, right.ROW), min(this->COL, right.COL) )の範囲
			/// @tparam U rightの型
			/// @param[in] const U& right
			/// @retval bool true=同じ, false=異なる
			template<typename U>
			bool operator==(const U& right)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				bool is_equal = true;
				for (size_t r = 0; r < ROW_LOOP_COUNT && is_equal; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT && is_equal; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						is_equal &= equal( this->get(r, c), static_cast<T>(getter::func(right, r, c)) );
					}
				}
				return is_equal;
			}

			/// @brief 同値でないかの判定
			///
			/// 同値判定で使用する型はthisの型になる
			/// 判定を行う範囲は( min(this->ROW, right.ROW), min(this->COL, right.COL) )になる
			/// @tparam U rightの型
			/// @param[in] const U& right
			/// @retval bool true=同じ, false=異なる
			template<typename U>
			bool operator!=(const U& right)const noexcept
			{
				return !(*this == right);
			}

			/// @brief 近似比較判定
			///
			/// 同値判定で使用する型はthisの型
			/// 判定を行う範囲は( min(this->ROW, right.ROW), min(this->COL, right.COL) )の範囲
			/// @tparam U rightの型
			/// @param[in] const U& right
			/// @param[in] const U& epsilon
			/// @retval bool true=同じ, false=異なる
			template<typename U>
			bool nearlyEqual(const U& right, const type& epsilon)const noexcept
			{
				static const size_t ROW_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_row<buffer, U>, meta::_get_row<buffer, U>>::type::func(*this, right);
				static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<buffer, U>, meta::_get_col<buffer, U>>::type::func(*this, right);
				bool is_equal = true;
				for (size_t r = 0; r < ROW_LOOP_COUNT && is_equal; ++r) {
					for (size_t c = 0; c < COL_LOOP_COUNT && is_equal; ++c) {
						using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
						is_equal &= math::nearlyEqual(this->get(r, c), static_cast<T>(getter::func(right, r, c)), epsilon);
					}
				}
				return is_equal;
			}

			/// @brief 現在の値を文字列に変換する
			/// @retval std::string
			std::string toString()const
			{
				std::string result = "";
				for (size_t r = 0; r < _row; ++r) {
					result += "(";
					for (size_t c = 0; c < _col; ++c) {
						result += std::to_string(this->get(r, c)) + ( (c+1 < _col) ? "," : "");
					}
					result += ")";
				}
				return result;
			}

		private:
			T m[_row][_col];
		};

		template struct buffer<float, 1, 2>;
		template struct buffer<int32_t, 1, 2>;
		template struct buffer<uint32_t, 1, 2>;

		/// @brief 内積計算
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T::type
		template<typename T, typename U>
		typename T::type dot(const T& left, const U& right)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::dot : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::dot : T and U ROW isn't 1...");
			static_assert(T::COL == U::COL, "ERROR math::dot : T and U isn't the same COL...");
			static const size_t COL_LOOP_COUNT = std::conditional<meta::_is_has_buffer<U>::value, meta::_compare_min_col<T, U>, meta::_get_col<T, U>>::type::func(left, right);
			float result = 0.f;
			for (size_t c = 0; c < COL_LOOP_COUNT; ++c) {
				using getter = std::conditional<meta::_is_has_buffer<U>::value, meta::_get_buffer_value<U>, meta::_get_value<U>>::type;
				result += getter::func(left, 0, c) * getter::func(right, 0, c);
			}
			return result;
		}

		/// @brief 2次元ベクトルの外積計算
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T::type
		template<typename T, typename U>
		typename T::type cross2D(const T& left, const U& right)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::cross : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::cross : T and U ROW isn't 1...");
			static_assert(T::COL == 2 && U::COL == 2, "ERROR math::cross : T and U COL isn't 2...");
			return left.buf.get<0, 0>() * right.buf.get<0, 1>() - left.buf.get<0, 1>() * right.buf.get<0, 0>();
		}

		/// @brief 3次元ベクトルの外積計算
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T
		template<typename T, typename U>
		T cross(const T& left, const U& right)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::cross : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::cross : T and U ROW isn't 1...");
			static_assert(T::COL == 3 && U::COL == 3, "ERROR math::cross : T and U COL isn't 3...");
			T result;
			result.buf.get<0, eX>() = left.buf.get<0, eY>() * right.buf.get<0, eZ>() - left.buf.get<0, eZ>() * right.buf.get<0, eY>();
			result.buf.get<0, eY>() = left.buf.get<0, eZ>() * right.buf.get<0, eX>() - left.buf.get<0, eX>() * right.buf.get<0, eZ>();
			result.buf.get<0, eZ>() = left.buf.get<0, eX>() * right.buf.get<0, eY>() - left.buf.get<0, eY>() * right.buf.get<0, eX>();
			return result;
		}

		/// @brief ベクトルの長さの2乗を計算する
		/// @tparam T
		/// @param[in] vec
		/// @retval T
		template<typename T>
		typename T::type lengthSq(const T& vec)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::lengthSq : T don't have math::buffer type...");
			static_assert(T::ROW == 1, "ERROR math::lengthSq : T ROW isn't 1...");
			float result = 0;
			for (size_t c = 0u; c < T::COL; ++c) {
				result += vec.buf.get(0, c) * vec.buf.get(0, c);
			}
			return result;
		}

		/// @brief ベクトルの長さを計算する
		/// @tparam T
		/// @param[in] vec
		/// @retval T
		template<typename T>
		typename T::type length(const T& vec)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::length : T don't have math::buffer type...");
			static_assert(T::ROW == 1, "ERROR math::length : T ROW isn't 1...");
			return sqrt(lengthSq(vec));
		}

		/// @brief ベクトルを正規化したものを返す
		/// @tparam T
		/// @param[in] vec
		/// @retval T
		template<typename T>
		T normalize(const T& vec)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::normalize : T don't have math::buffer type...");
			static_assert(T::ROW == 1, "ERROR math::cross : T ROW isn't 1...");
			using namespace std;
			T::type len = std::max(length(vec), 0.00000001f);//NaNにしたほうがいい？
			return vec / len;
		}

		/// @brief ベクトルの正規化
		/// @tparam T
		/// @param[in] vec
		/// @retval T
		template<typename T>
		T& normalized(T& vec)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::normalized : T don't have math::buffer type...");
			static_assert(T::ROW == 1, "ERROR math::cross : T ROW isn't 1...");
			using namespace std;
			T::type len = std::max(length(vec), 0.00000001f);//NaNにしたほうがいい？
			return (vec /= len);
		}

		/// @brief minValueからmaxValueの範囲に要素を設定する
		/// todo float型やdouble型にも対応する
		/// @tparam T
		/// @param[in] x
		/// @retval T
		template<typename T>
		T clamp(T& x, typename T::type minValue, typename T::type maxValue)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::saturate : T don't have math::buffer type...");
			using namespace std;
			T result = x;
			result.buf.map([&](decltype(result.buf)& self, size_t row, size_t col) {
				using namespace std;
				return min(max(self.get(row, col), static_cast<T::type>(minValue)), static_cast<T::type>(maxValue));
			});
			return result;
		}

		/// @brief 0から1の範囲にクランプする
		/// @tparam T
		/// @param[in] x
		/// @retval T
		template<typename T>
		T saturate(T& x)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::saturate : T don't have math::buffer type...");
			return clamp(x, 0, 1);
		}

		/// @brief 乱数を設定する
		/// todo C++11の乱数生成を対応する
		/// @tparam T
		/// @param[in] x
		/// @param[in] min_
		/// @param[in] max_
		/// @retval T
		template<typename T>
		typename T setRandom(typename T::type min_, typename T::type max_)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::random : T don't have math::buffer type...");
			using namespace std;
			T result;
			result.buf.map([&](decltype(result.buf)& self, size_t row, size_t col) {
				return simpleRandom(min_, max_);
			});
			return result;
		}

		/// @brief 線形補間(hinode::math::buffer関係の型)
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T
		template<typename T, typename U>
		typename T lerp(const T& left, const U& right, float t, typename std::enable_if<meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value&& !(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value)>::type* = 0)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::lerp : T and U don't have math::buffer type...");
			static_assert(T::ROW == U::ROW, "ERROR math::lerp : T and U ROW don't equal...");
			static_assert(T::COL == U::COL, "ERROR math::lerp : T and U COL don't equal...");
			return left + t * (right - left);
		}

		/// @brief 線形補間(算術型)
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T
		template<typename T, typename U>
		typename T lerp(const T& left, const U& right, float t, typename std::enable_if<std::is_arithmetic<T>::value && std::is_arithmetic<U>::value && !(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value)>::type* = 0)noexcept
		{
			return left + static_cast<T>(t * (right - left));
		}

		/// @brief ベクトル間の角度(ラジアン)計算
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T::type
		template<typename T, typename U>
		typename T::type calRadian(const T& left, const U& right)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::calRadian : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::calRadian : T and U ROW isn't 1...");
			static_assert(T::COL == U::COL, "ERROR math::calRadian : T and U COL don't equal...");
			static_assert(2 <= T::COL && T::COL <= 3, "ERROR math::calRadian : T COL range is invalid...");
			static_assert(2 <= U::COL && U::COL <= 3, "ERROR math::calRadian : U COL range is invalid...");

			T::type d = dot(left, right);
			T::type leftLen = length(left);
			T::type rightLen = length(right);

			T::type n = d / (leftLen*rightLen);
			using namespace std;
			n = max(min(n, 1.f), -1.f);
			return acos(n);
		}

		/// @brief 射影ベクトルの計算
		///
		///	leftをrightに射影したベクトルを返す
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] right
		/// @retval T::type
		template<typename T, typename U>
		typename T projection(const T& left, const U& right)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::projection : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::projection : T and U ROW isn't 1...");
			static_assert(T::COL == U::COL, "ERROR math::projection : T and U COL don't equal...");
			static_assert(2 <= T::COL && T::COL <= 3, "ERROR math::projection : T COL range is invalid...");
			static_assert(2 <= U::COL && U::COL <= 3, "ERROR math::projection : U COL range is invalid...");

			T::type d = dot(left, right);
			return right * d / lengthSq(right);
		}

		/// @brief 反射ベクトルの計算
		///
		///	leftをnormalで反射したベクトルを返す
		/// @tparam T
		/// @tparam U
		/// @param[in] left
		/// @param[in] normal 正規化したベクトルを渡してください
		/// @retval T::type
		template<typename T, typename U>
		typename T reflect(const T& left, const U& normal)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value && meta::_is_has_buffer<U>::value, "ERROR math::reflect : T and U don't have math::buffer type...");
			static_assert(T::ROW == 1 && U::ROW == 1, "ERROR math::reflect : T and U ROW isn't 1...");
			static_assert(T::COL == U::COL, "ERROR math::reflect : T and U COL don't equal...");
			static_assert(2 <= T::COL && T::COL <= 3, "ERROR math::reflect : T COL range is invalid...");
			static_assert(2 <= U::COL && U::COL <= 3, "ERROR math::reflect : U COL range is invalid...");

			T::type d = dot(left, normal);
			return left - 2*d*normal;
		}

		/// @brief scaler * math::buffer系の共通関数
		/// @tparam _Scaler
		/// @tparam _BufferType
		/// @param[in] _Scaler scaler
		/// @param[in] _BufferType right
		/// @retval _BufferType
		template<typename _Scaler, typename _BufferType>
		_BufferType operatorMul(_Scaler scaler, const _BufferType& right)noexcept
		{
			static_assert(std::is_arithmetic<_Scaler>::value, "T isn't numeric type.");
			static_assert(meta::_validate_buffer_type<_BufferType>::value, "_BufferType isn't buffer type.");
			_BufferType result = right;
			result.buf *= scaler;
			return result;
		}

		template<typename T> inline T operator*(int scaler, const T& right)noexcept
		{
			return operatorMul(scaler, right);
		}
		template<typename T> inline T operator*(unsigned int scaler, const T& right)noexcept
		{
			return operatorMul(scaler, right);
		}
		template<typename T> inline T operator*(float scaler, const T& right)noexcept
		{
			return operatorMul(scaler, right);
		}

		/// @brief 2次元ベクトル
		///
		///	シェーダソースと統一を取るため数学関数はメンバ関数に持たせていない
		template<typename T> struct Vector2
		{
			typedef Vector2 self_type;
			typedef T type;
			static const size_t ROW = 1;
			static const size_t COL = 2;

			union {
				buffer<T, ROW, COL> buf;
				struct {
					T x, y;
				};
				struct {
					T r, g;
				};
				T value[COL];
			};

			self_type()noexcept
				: x(0), y(0)
			{}

			self_type(T value)noexcept
				: x(value), y(value)
			{}

			self_type(T x, T y)noexcept
				: x(x), y(y)
			{}

			self_type(const self_type& original)noexcept
				: x(original.x), y(original.y)
			{}

			template<typename U> self_type(const U& original)noexcept
				: self_type()
			{
				this->buf = original;
			}

			self_type& operator()(T x, T y)noexcept
			{
				this->x = x;
				this->y = y;
				return *this;
			}

			operator auto ()noexcept -> decltype(buf)& { return this->buf; }
			operator auto ()const noexcept-> const decltype(buf)& { return this->buf; }

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type& operator=(const U& right)noexcept
			{
				this->buf = right;
				return *this;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator+(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf += right;
				return r;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator+=(const U& right)noexcept
			{
				this->buf += right;
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator-(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf -= right;
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator-=(const U& right)noexcept
			{
				this->buf -= right;
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator*(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf *= right;
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator*=(const U& right)noexcept
			{
				this->buf *= right;
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator/(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf /= right;
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator/=(const U& right)noexcept
			{
				this->buf /= right;
				return *this;
			}

			/// @brief 剰余演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator%(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf %= right;
				return r;
			}

			/// @brief 剰余代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator%=(const U& right)noexcept
			{
				this->buf %= right;
				return *this;
			}

			/// @brief 同値判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator==(const U& right)const noexcept
			{
				return this->buf == right;
			}

			/// @brief 同値でないかの判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator!=(const U& right)const noexcept
			{
				return this->buf != right;
			}
		};

		typedef Vector2<float> float2;
		typedef Vector2<float> vec2;
		typedef Vector2<int32_t> int2;
		typedef Vector2<int32_t> ivec2;
		typedef Vector2<uint32_t> uint2;
		typedef Vector2<uint32_t> uvec2;

		/// @brief 3次元ベクトル
		///
		///	シェーダソースと統一を取るため数学関数はメンバ関数に持たせていない
		template<typename T> struct Vector3
		{
			typedef Vector3 self_type;
			typedef T type;
			static const size_t ROW = 1;
			static const size_t COL = 3;

			union {
				buffer<T, ROW, COL> buf;
				struct {
					T x, y, z;
				};
				struct {
					T r, g, b;
				};
				T value[COL];
			};

			self_type()noexcept
				: x(0), y(0), z(0)
			{}

			self_type(T value)noexcept
				: x(value), y(value), z(value)
			{}

			self_type(T x, T y, T z)noexcept
				: x(x), y(y), z(z)
			{}

			self_type(const self_type& original)noexcept
				: x(original.x), y(original.y), z(original.z)
			{}

			template<typename U>
			self_type(const Vector2<U>& vec, T z)noexcept
				: x(vec.x), y(vec.y), z(z)
			{}

			template<typename U> self_type(const U& original)noexcept
				: self_type()
			{
				this->buf = original;
			}

			self_type& operator()(T x, T y, T z)noexcept
			{
				this->x = x;
				this->y = y;
				this->z = z;
				return *this;
			}

			operator auto ()noexcept -> decltype(buf)& { return this->buf; }
			operator auto ()const noexcept-> const decltype(buf)& { return this->buf; }

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type& operator=(const U& right)noexcept
			{
				this->buf = right;
				return *this;
			}

			/// @brief 単項マイナス演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			self_type operator-()const noexcept
			{
				self_type result;
				result.buf = -this->buf;
				return result;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type operator+(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf += right;
				return r;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type&
			template<typename U> self_type& operator+=(const U& right)noexcept
			{
				this->buf += right;
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type operator-(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf -= right;
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type&
			template<typename U> self_type& operator-=(const U& right)noexcept
			{
				this->buf -= right;
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type operator*(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf *= right;
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type&
			template<typename U> self_type& operator*=(const U& right)noexcept
			{
				this->buf *= right;
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type operator/(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf /= right;
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type&
			template<typename U> self_type& operator/=(const U& right)noexcept
			{
				this->buf /= right;
				return *this;
			}

			/// @brief 剰余演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			template<typename U> self_type operator%(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf %= right;
				return r;
			}

			/// @brief 剰余代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type&
			template<typename U> self_type& operator%=(const U& right)noexcept
			{
				this->buf %= right;
				return *this;
			}

			/// @brief 同値判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator==(const U& right)const noexcept
			{
				return this->buf == right;
			}

			/// @brief 同値でないかの判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator!=(const U& right)const noexcept
			{
				return this->buf != right;
			}
		};
		using float3 = Vector3<float>;
		using vec3 = Vector3<float>;
		using int3 = Vector3<int>;
		using ivec3 = Vector3<int>;
		using uint3 = Vector3<unsigned int>;
		using uvec3 = Vector3<unsigned int>;

		/// @brief 4次元ベクトル
		///
		///	シェーダソースと統一を取るため数学関数はメンバ関数に持たせていない
		template<typename T> struct Vector4
		{
			typedef Vector4 self_type;
			typedef T type;
			static const size_t ROW = 1;
			static const size_t COL = 4;

			union {
				buffer<T, ROW, COL> buf;
				struct {
					T x, y, z, w;
				};
				struct {
					T r, g, b, a;
				};
				T value[COL];
			};

			self_type()noexcept
				: x(0), y(0), z(0), w(1)
			{}

			self_type(T value)noexcept
				: x(value), y(value), z(value), w(value)
			{}

			self_type(T x, T y, T z, T w)noexcept
				: x(x), y(y), z(z), w(w)
			{}

			template<typename U>
			self_type(const Vector2<U>& vec, T z, T w)noexcept
				: x(vec.x) , y(vec.y) , z(z) , w(w)
			{}

			template<typename U>
			self_type(const Vector3<U>& vec, T w)noexcept
				: x(vec.x) , y(vec.y), z(vec.z), w(w)
			{}

			self_type(const self_type& original)noexcept
				: x(original.x), y(original.y), z(original.z), w(original.w)
			{}

			template<typename U> self_type(const U& original)noexcept
				: self_type()
			{
				this->buf = original;
			}

			self_type& operator()(T x, T y, T z, T w)noexcept
			{
				this->x = x;
				this->y = y;
				this->z = z;
				this->w = w;
				return *this;
			}

			operator auto ()noexcept -> decltype(buf)& { return this->buf; }
			operator auto ()const noexcept-> const decltype(buf)& { return this->buf; }

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type& operator=(const U& right)noexcept
			{
				this->buf = right;
				return *this;
			}

			/// @brief 単項マイナス演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type
			self_type operator-()const noexcept
			{
				self_type result;
				result.buf = -this->buf;
				return result;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator+(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf += right;
				return r;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator+=(const U& right)noexcept
			{
				this->buf += right;
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator-(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf -= right;
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator-=(const U& right)noexcept
			{
				this->buf -= right;
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator*(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf *= right;
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator*=(const U& right)noexcept
			{
				this->buf *= right;
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator/(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf /= right;
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator/=(const U& right)noexcept
			{
				this->buf /= right;
				return *this;
			}

			/// @brief 剰余演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator%(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf %= right;
				return r;
			}

			/// @brief 剰余代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator%=(const U& right)noexcept
			{
				this->buf %= right;
				return *this;
			}

			/// @brief 同値判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator==(const U& right)const noexcept
			{
				return this->buf == right;
			}

			/// @brief 同値でないかの判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator!=(const U& right)const noexcept
			{
				return this->buf != right;
			}
		};
		typedef Vector4<float> float4;
		typedef Vector4<float> vec4;
		typedef Vector4<int> int4;
		typedef Vector4<int> ivec4;
		typedef Vector4<unsigned int> uint4;
		typedef Vector4<unsigned int> uvec4;
		typedef Vector4<float> quaternion;

		/// @brief float4の値を32bit幅のカラーに変換する
		/// @param color
		/// @retval uint32_t
		inline uint32_t convertToColor(const float4& color)
		{
			uint32_t result = 0;
			result |= (static_cast<uint32_t>(color.a * 255) << 24);
			result |= (static_cast<uint32_t>(color.r * 255) << 16);
			result |= (static_cast<uint32_t>(color.g * 255) << 8);
			result |= (static_cast<uint32_t>(color.b * 255));
			return result;
		}
		/// @brief 32bit幅のカラーをfloat4に変換する
		/// @param color
		/// @retval Vector
		inline float4 convertToColor(const uint32_t& color)
		{
			return{
				((color >> 16) & 0xff) / 255.f,
				((color >> 8) & 0xff) / 255.f,
				((color)& 0xff) / 255.f,
				((color >> 24) & 0xff) / 255.f,
			};
		}

		/// @brief 3x3行列
		///
		///	シェーダソースと統一を取るため数学関数はメンバ関数に持たせていない
		template<typename T> struct Matrix3x3
		{
			typedef Matrix3x3 self_type;
			typedef T type;
			static const size_t ROW = 3;
			static const size_t COL = 3;

			union {
				buffer<T, ROW, COL> buf;
				Vector3<T> row[ROW];
				struct {
					T _11, _12, _13;
					T _21, _22, _23;
					T _31, _32, _33;
				};
			};

			self_type()noexcept : self_type(0.f)
			{
			}

			self_type(T value)noexcept
			{
				this->buf = value;
			}

			self_type(const self_type& original)noexcept : buf(original)
			{
			}

			template<typename U> self_type(const U& original)noexcept
				: self_type()
			{
				this->buf = original;
			}



			operator auto ()noexcept -> decltype(buf)& { return this->buf; }
			operator auto ()const noexcept-> const decltype(buf)& { return this->buf; }

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type& operator=(const U& right)noexcept
			{
				this->buf = right;
				return *this;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator+(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf += right;
				return r;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator+=(const U& right)noexcept
			{
				this->buf += right;
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator-(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf -= right;
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator-=(const U& right)noexcept
			{
				this->buf -= right;
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator*(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf *= right;
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator*=(const U& right)noexcept
			{
				this->buf *= right;
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator/(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf /= right;
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator/=(const U& right)noexcept
			{
				this->buf /= right;
				return *this;
			}

			/// @brief 剰余演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator%(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf %= right;
				return r;
			}

			/// @brief 剰余代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator%=(const U& right)noexcept
			{
				this->buf %= right;
				return *this;
			}

			/// @brief 同値判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator==(const U& right)const noexcept
			{
				return this->buf == right;
			}

			/// @brief 同値でないかの判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator!=(const U& right)const noexcept
			{
				return this->buf != right;
			}
		};
		typedef Matrix3x3<float> float3x3;
		typedef Matrix3x3<float> mat3;

		/// @brief 4x4行列
		///
		///	シェーダソースと統一を取るため数学関数はメンバ関数に持たせていない
		template<typename T> struct Matrix4x4
		{
			typedef Matrix4x4 self_type;
			typedef T type;
			static const size_t ROW = 4;
			static const size_t COL = 4;

			union {
				buffer<T, ROW, COL> buf;
				Vector4<T> row[ROW];
				struct {
					T _11, _12, _13, _14;
					T _21, _22, _23, _24;
					T _31, _32, _33, _34;
					T _41, _42, _43, _44;
				};
			};

			self_type()noexcept : self_type(0.f)
			{
			}

			self_type(T value)noexcept
			{
				this->buf = value;
			}

			self_type(const self_type& original)noexcept : buf(original)
			{
			}

			template<typename U> self_type(const U& original)noexcept
				: self_type()
			{
				this->buf = original;
			}



			operator auto ()noexcept -> decltype(buf)& { return this->buf; }
			operator auto ()const noexcept-> const decltype(buf)& { return this->buf; }

			/// @brief 添字アクセス
			/// @param[in] rowIndex
			/// @retval float4&
			float4& operator[](size_t rowIndex)noexcept
			{
				assert(rowIndex < this->ROW && "math::Matrix4x4::operator[] : invalid row index!");
				return this->row[rowIndex];
			}

			/// @brief 添字アクセス
			/// @param[in] rowIndex
			/// @retval const float4&
			const float4& operator[](size_t rowIndex)const noexcept
			{
				assert(rowIndex < this->ROW && "math::Matrix4x4::operator[] : invalid row index!");
				return this->row[rowIndex];
			}

			/// @brief 代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type& operator=(const U& right)noexcept
			{
				this->buf = right;
				return *this;
			}

			/// @brief 加算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator+(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf += right;
				return r;
			}

			/// @brief 加算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator+=(const U& right)noexcept
			{
				this->buf += right;
				return *this;
			}

			/// @brief 減算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator-(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf -= right;
				return r;
			}

			/// @brief 減算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator-=(const U& right)noexcept
			{
				this->buf -= right;
				return *this;
			}

			/// @brief 乗算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator*(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf *= right;
				return r;
			}

			/// @brief 乗算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator*=(const U& right)noexcept
			{
				this->buf *= right;
				return *this;
			}

			/// @brief 除算演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator/(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf /= right;
				return r;
			}

			/// @brief 除算代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator/=(const U& right)noexcept
			{
				this->buf /= right;
				return *this;
			}

			/// @brief 剰余演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>
			template<typename U> self_type operator%(const U& right)const noexcept
			{
				self_type r = *this;
				r.buf %= right;
				return r;
			}

			/// @brief 剰余代入演算子のオーバーロード
			/// @tparam U
			/// @param[in] right
			/// @retval self_type<T>&
			template<typename U> self_type& operator%=(const U& right)noexcept
			{
				this->buf %= right;
				return *this;
			}

			/// @brief 同値判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator==(const U& right)const noexcept
			{
				return this->buf == right;
			}

			/// @brief 同値でないかの判定
			///
			/// @tparam U
			/// @param[in] right
			/// @retval bool true=同じ, false=異なる
			template<typename U> bool operator!=(const U& right)const noexcept
			{
				return this->buf != right;
			}
		};
		typedef Matrix4x4<float> matrix;
		typedef Matrix4x4<float> float4x4;
		typedef Matrix4x4<float> mat4;

		/// @brief 単位行列を設定する
		/// @tparam T
		/// @param[in] mat
		/// @retval T
		template<typename T>
		T& setIdentity(T& mat)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::identity : T don't have math::buffer...");
			static_assert(T::ROW == T::COL, "ERROR math::identity : T isn't square matrix...");
			mat.buf.map([](decltype(mat.buf)& self, size_t row, size_t col) {
				return static_cast<typename T::type>(row == col ? 1 : 0);
			});
			return mat;
		}

		/// @brief 転置行列を設定する
		/// @tparam T
		/// @param[in] mat
		/// @retval T
		template<typename T>
		T& setTranspose(T& mat)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::transpose : T don't have math::buffer...");
			static_assert(T::ROW == T::COL, "ERROR math::transpose : T isn't square matrix...");
			for (size_t r = 0; r < T::ROW; ++r) {
				for (size_t c = r+1; c < T::COL; ++c) {
					std::swap(mat.buf.get(r, c), mat.buf.get(c, r));
				}
			}
			return mat;
		}

		/// @brief 転置行列を求める
		/// @tparam T
		/// @param[in] mat
		/// @retval T
		template<typename T>
		T transpose(const T& mat)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::transpose : T don't have math::buffer...");
			static_assert(T::ROW == T::COL, "ERROR math::transpose : T isn't square matrix...");
			T result = mat;
			for (size_t r = 0; r < T::ROW; ++r) {
				for (size_t c = r + 1; c < T::COL; ++c) {
					std::swap(result.buf.get(r, c), result.buf.get(c, r));
				}
			}
			return result;
		}

		/// @brief 3x3行列の行列式を計算する
		/// @tparam T
		/// @param[in] mat
		/// @retval T
		template<typename T>
		T determinant(const Matrix3x3<T>& mat)noexcept
		{
			std::array<size_t, 3> permutation = { {
				0, 1, 2
			} };
			float value = mat.buf.get(0, permutation[0]) * mat.buf.get(1, permutation[1]) * mat.buf.get(2, permutation[2]);
			std::next_permutation(permutation.begin(), permutation.end());

			int sgnIndex = 0;
			do {
				float sign = ((sgnIndex & 0x3) < 0x2 ? -1.f : 1.f);
				value += sign * mat.buf.get(0, permutation[0]) * mat.buf.get(1, permutation[1]) * mat.buf.get(2, permutation[2]);
				sgnIndex++;
			} while (std::next_permutation(permutation.begin(), permutation.end()));
			return value;
		}

		/// @brief 4x4行列の行列式を計算する
		/// @tparam T
		/// @param[in] mat
		/// @retval T
		template<typename T>
		T determinant(const Matrix4x4<T>& mat)noexcept
		{
			std::array<size_t, 4> permutation = { {
					0, 1, 2, 3
				} };
			float value = mat.buf.get(0, permutation[0]) * mat.buf.get(1, permutation[1]) * mat.buf.get(2, permutation[2]) * mat.buf.get(3, permutation[3]);
			std::next_permutation(permutation.begin(), permutation.end());

			int sgnIndex = 0;
			do {
				float sign = ((sgnIndex & 0x3) < 0x2 ? -1.f : 1.f);
				value += sign * mat.buf.get(0, permutation[0]) * mat.buf.get(1, permutation[1]) * mat.buf.get(2, permutation[2]) * mat.buf.get(3, permutation[3]);
				sgnIndex++;
			} while (std::next_permutation(permutation.begin(), permutation.end()));
			return value;
		}


		/// @brief 行列のLU分解を行う
		///
		///	参考サイト：https://en.wikipedia.org/wiki/LU_decomposition
		/// @tparam T
		/// @param[out] pOutLU
		/// @param[out] permutationIndices 行の置換情報。LU分解の際、計算誤差を少なくするために行の置換を行うので、各行のもともとの行番号を記録している
		/// @param[in] mat
		/// @param[in] tolerance default=0 誤差の許容範囲。行列の値によっては誤差が大きくなって正しい結果を求められない場合があるので、それを検出するためのもの
		/// @retval T
		template<typename T>
		bool LUDecompose(T* pOutLU, std::array<size_t, T::ROW + 1>& permutationIndices, const T& mat, typename T::type tolerance = 0)noexcept
		{
			static_assert(T::ROW == T::COL, "ERROR math::LUDecomposition : T isn't square matrix...");
			assert(nullptr != pOutLU);
			*pOutLU = mat;

			for (auto i = 0u; i < permutationIndices.size(); ++i) {
				permutationIndices[i] = i; //Unit permutation matrix, P[N] initialized with N 
			}
			std::array<float*, T::ROW> pRow;
			for (auto i = 0u; i < pRow.size(); ++i) {
				pRow[i] = pOutLU->buf.getRow(i);
			}

			for (size_t row = 0; row<T::ROW; ++row) {
				T::type maxA = 0.0;
				size_t maxRowIndex = row;
				for (auto k = row; k < T::ROW; ++k) {
					auto v = fabs(pRow[k][row]);
					if (maxA < v) {
						maxA = v;
						maxRowIndex = k;
					}
				}

				if (maxA < tolerance) {
					return false;
				}

				if (maxRowIndex != row) {
					std::swap(permutationIndices[row], permutationIndices[maxRowIndex]);
					std::swap(pRow[row], pRow[maxRowIndex]);
					permutationIndices[T::ROW]++; //counting pivots starting from N (for determinant)
				}

				for (auto j = row + 1; j<T::ROW; ++j) {
					pRow[j][row] /= pRow[row][row];
					for (auto k = row + 1; k < T::ROW; ++k) {
						pRow[j][k] -= pRow[j][row] * pRow[row][k];
					}
				}
			}
			return true;  //decomposition done 
		}

		/// @brief LU分解をした行列の逆行列を求める
		///
		///	参考サイト：https://en.wikipedia.org/wiki/LU_decomposition
		/// この関数に渡す引数はLUDecompose関数で生成したものにしてください。
		/// @tparam T
		/// @param[in] LU LU分解後の行列
		/// @param[in] permutationIndices 行の置換情報
		/// @retval T
		template<typename T>
		T inverse(const T& LU, std::array<size_t, T::ROW + 1>& permutationIndices)noexcept
		{
			static_assert(T::ROW == T::COL, "ERROR math::inverse : T isn't square matrix...");
			T result;
			for (int col = 0; col<T::ROW; col++) {
				for (int row = 0; row<T::ROW; row++) {
					if (permutationIndices[row] == col)
						result.buf.get(row, col) = 1.0;
					else
						result.buf.get(row, col) = 0.0;
					for (int k = 0; k < row; k++) {
						result.buf.get(row, col) -= LU.buf.get(permutationIndices[row], k) * result.buf.get(k, col);
					}
				}
				for (int row = T::ROW - 1; row >= 0; row--) {
					for (int k = row + 1; k < T::ROW; k++) {
						result.buf.get(row, col) -= LU.buf.get(permutationIndices[row], k) * result.buf.get(k, col);
					}
					result.buf.get(row, col) = result.buf.get(row, col) / LU.buf.get(permutationIndices[row], row);
				}
			}
			return result;
		}

		/// @brief LU分解した行列の行列式を求める
		///
		///	参考サイト：https://en.wikipedia.org/wiki/LU_decomposition
		/// この関数に渡す引数はLUDecompose関数で生成したものにしてください。
		/// @tparam T
		/// @param[in] LU LU分解後の行列
		/// @param[in] permutationIndices 行の置換情報
		/// @retval T
		template<typename T>
		typename T::type determinant(const T& LU, const std::array<size_t, T::ROW+1>& permutationIndices)
		{
			typename T::type det = LU.buf.get(0, 0);
			for (auto i = 1; i < T::ROW; ++i) {
				det *= LU.buf.get(i, i);
			}
			if (0 == (permutationIndices[T::ROW] - T::ROW) % 2) {
				return det;
			} else {
				return -det;
			}
		}

		/// @brief 行列の逆行列を求める
		///
		///	参考サイト：https://en.wikipedia.org/wiki/LU_decomposition
		/// 逆行列がない場合や求められなかったときはmatを返します
		/// @tparam T
		/// @param[in] mat
		/// @param[in] tolerance default=0 誤差の許容範囲。行列の値によっては誤差が大きくなって正しい結果を求められない場合があるので、それを検出するためのもの
		/// @param[out] isSuccess 逆行列が求められたときはtrueが設定される
		/// @retval T
		template<typename T>
		T inverse(const T& mat, typename T::type tolerance = 0, bool* pOutIsSuccess=nullptr)noexcept
		{
			static_assert(T::ROW == T::COL, "ERROR math::inverse : T isn't square matrix...");
			if(pOutIsSuccess) *pOutIsSuccess = false;

			T LU;
			std::array<size_t, T::ROW + 1> permutationIndices;
			if (!LUDecompose(&LU, permutationIndices, mat, tolerance)) {
				return mat;
			}

			auto det = determinant(LU, permutationIndices);
			if (0 == det) {
				return mat;
			}
			if (pOutIsSuccess) *pOutIsSuccess = true;
			return inverse(LU, permutationIndices);
		}

		/// @brief 行列の掛け算を行う
		///
		///	todo mul(行列, ベクトル)の対応。(現状、ベクトルと行列の積はベクトル * 行列にしか対応していない)
		/// @tparam T
		/// @tparam U
		/// @param left
		/// @param right
		/// @retval T
		template<typename T, typename U>
		T mul(const T& left, const U& right)
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::mul : T don't have math::buffer...");
			static_assert(meta::_is_has_buffer<U>::value, "ERROR math::mul : U don't have math::buffer...");
			static_assert(T::COL == U::ROW, "ERROR math::mul : T and U don't multiply...");

			T result;
			result.buf.map([&](decltype(result.buf)& self, size_t row, size_t col) {
				T::type value=0.f;
				for (size_t i = 0; i < T::COL; ++i) {
					value += left.buf.get(row, i) * right.buf.get(i, col);
				}
				return value;
			});
			return result;
		}

		/// @brief X軸回転行列を生成する
		/// @tparam T
		/// @param radian
		/// @retval T
		template<typename T>
		T makeRotationX(float radian)
		{
			T result;
			setIdentity(result);
			const float s = sin(radian);
			const float c = cos(radian);
			result._22 = c; result._23 = s;
			result._32 = -s; result._33 = c;
			return result;
		}

		/// @brief Y軸回転行列を生成する
		/// @tparam T
		/// @param radian
		/// @retval T
		template<typename T>
		T makeRotationY(float radian)
		{
			T result;
			setIdentity(result);
			const float s = sin(radian);
			const float c = cos(radian);
			result._11 = c; result._13 = -s;
			result._31 = s; result._33 = c;
			return result;
		}

		/// @brief Z軸回転行列を生成する
		/// @tparam T
		/// @param radian
		/// @retval T
		template<typename T>
		T makeRotationZ(float radian)
		{
			T result;
			setIdentity(result);
			const float s = sin(radian);
			const float c = cos(radian);
			result._11 = c; result._12 = s;
			result._21 = -s; result._22 = c;
			return result;
		}

		/// @brief 任意軸回転行列を生成する
		/// @tparam T
		/// @param axis
		/// @param radian
		/// @retval T
		template<typename T>
		T makeRotationAxis(const float3& axis, float radian)
		{
			T result;
			setIdentity(result);
			const float Sin = sin(radian);
			const float Cos = cos(radian);
			const float _1MinasCos = 1 - Cos;

			float Axy = axis.x * axis.y;
			float Ayz = axis.y * axis.z;
			float Azx = axis.z * axis.x;
			//メモ: s => squareの略　２乗を表している
			float sAx = axis.x*axis.x;
			float sAy = axis.y*axis.y;
			float sAz = axis.z*axis.z;

			float AxSin = axis.x * Sin;
			float AySin = axis.y * Sin;
			float AzSin = axis.z * Sin;

			result._11 = sAx*_1MinasCos + Cos;   result._12 = Axy*_1MinasCos + AzSin;	result._13 = Azx*_1MinasCos - AySin;
			result._21 = Axy*_1MinasCos - AzSin; result._22 = sAy*_1MinasCos + Cos;		result._23 = Ayz*_1MinasCos + AxSin;
			result._31 = Azx*_1MinasCos + AySin; result._32 = Ayz*_1MinasCos - AxSin;	result._33 = sAz*_1MinasCos + Cos;

			return result;
		}

		/// @brief 指定した3軸を向く回転行列を生成する
		/// @tparam T
		/// @param front
		/// @param up
		/// @param right
		/// @retval T
		template<typename T>
		T makeRotationAxisXYZ(const float3& front, const float3& up, const float3& right)
		{
			T result;
			setIdentity(result);
			result.row[0] = right;
			result.row[1] = up;
			result.row[2] = front;
			return result;
		}

		/// @brief 平行移動行列を生成する
		/// @tparam T
		/// @param radian
		/// @retval T
		template<typename T>
		float4x4 makeTranslation(const T& pos)
		{
			static_assert(T::COL <= 3, "ERROR math::makeTranslation : T COL must be less 3...");
			float4x4 result;
			setIdentity(result);
			for (size_t i = 0; i < pos.COL; ++i) {
				result.buf.get(3, i) = pos.buf.get(0, i);
			}
			return result;
		}

		/// @brief 拡大行列を生成する
		/// 使用するときはテンプレート引数を明示的に指定してください
		/// @tparam T
		/// @param scale
		/// @retval T
		template<typename T>
		T makeScale(const Vector3<float>& scale)
		{
			T result;
			setIdentity(result);
			result._11 = scale.x;
			result._22 = scale.y;
			result._33 = scale.z;
			return result;
		}
		///// @brief 拡大行列を生成する
		///// @tparam T
		///// @param scale
		///// @retval T
		//template<typename T>
		//T makeScale(const Vector3<float>&& scale)
		//{
		//	return makeScale(scale);
		//}

		/// @brief ビュー行列を生成する
		/// @param pos カメラの位置
		/// @param target 注視点
		/// @param up カメラの上方向
		/// @retval float4x4
		inline float4x4 makeView(const Vector3<float>& pos, const Vector3<float>& target, const Vector3<float>& up)
		{
			auto front = normalize(target - pos);
			auto right = normalize(cross(up, front));
			auto _up = normalize(cross(front, right));
			float3 d;
			d.x = dot(right, pos);
			d.y = dot(up, pos);
			d.z = dot(front, pos);

			float4x4 result;
			setIdentity(result);
			result.row[0](right.x, _up.x, front.x, 0);
			result.row[1](right.y, _up.y, front.y, 0);
			result.row[2](right.z, _up.z, front.z, 0);
			result.row[3](-d.x, -d.y, -d.z, 1);
			return result;
		}

		/// @brief 投影行列を生成する
		/// @param nearZ 
		/// @param farZ 
		/// @param radian 画角
		/// @param aspect アスペクト比
		/// @retval float4x4
		inline float4x4 makePerspective(float nearZ, float farZ, float radian, float aspect)
		{
			float sY = 1.f / tan(radian/2.f);
			float sX = sY / aspect;
			float work = farZ / (farZ - nearZ);
			float4x4 result;
			setIdentity(result);
			result.row[0](sX, 0, 0, 0);
			result.row[1](0, sY, 0, 0);
			result.row[2](0, 0, work, 1);
			result.row[3](0, 0, -nearZ*work, 0);
			return result;
		}

		/// @brief 平行投影行列を生成する
		/// @param[in] width
		/// @param[in] height
		/// @param[in] nezrZ
		/// @param[in] farZ
		/// @retval float4x4
		inline float4x4 makeOrthogonal(float width, float height, float nearZ, float farZ)
		{
			float2 size = float2(width, height) * 0.5f;
			float zRange = farZ - nearZ;
			float4x4 result;
			result.row[0](2 / width, 0, 0, 0);
			result.row[1](0, 2 / height, 0, 0);
			result.row[2](0, 0, 1 / zRange, 0);
			result.row[3](0, 0, -nearZ/zRange, 1);
			return result;
		}

		/// @brief クリッピング空間からUV空間へ変換する行列を生成する
		/// @retval float4x4
		inline float4x4 makeToUVFromClipSpace()
		{
			float4x4 result;
			result.row[0](0.5f, 0, 0, 0);
			result.row[1](0, -0.5f, 0, 0);
			result.row[2](0, 0, 1, 0);
			result.row[3](0.5f, 0.5f, 0, 1);
			return result;
		}
		/// @brief ビューポート行列を生成する
		/// @param width
		/// @param height
		/// @retval float4x4
		inline float4x4 makeViewport(float width, float height)
		{
			float4x4 result;
			setIdentity(result);
			result._11 = width * 0.5f;
			result._22 = -height * 0.5f;
			result._41 = result._11;
			result._42 = result._22;
			return result;
		}

		/// @brief 回転軸axisでradia回転を表すクォータニオンを生成する
		/// @param[in] axis 正規化してください
		/// @param[in] radian
		/// @retval quaternion
		inline quaternion makeRotaQuaternion(const float3& axis, float radian)noexcept
		{
			radian *= 0.5f;
			float s = sin(radian);
			float c = cos(radian);
			quaternion result(axis * s, c);
			return result;
		}

		/// @brief baseDirからtargetDirへの回転を表すクォータニオンを生成する
		/// @param[in] baseDir
		/// @param[in] secondDir
		/// @retval quaternion
		inline quaternion makeRotaQuaternion(const float3& baseDir, const float3& targetDir)noexcept
		{
			assert(baseDir != float3(0, 0, 0) && targetDir != float3(0, 0, 0));
			float d = dot(baseDir, targetDir);
			if (d <= -1.f || 1.f <= d) {
				//特殊パターン
				quaternion result;
				result(0, 0, 0, 1);//同じ方向ならなにもしないようにしている
				if (0.f < d) {
					//正反対のときはそれぞれの方向に直角な軸を求め回転させる
					float3 workUp(0, 1, 0);
					if (hinode::math::nearlyEqual(baseDir.x, 0.f) && hinode::math::nearlyEqual(baseDir.z, 0.f)) {//もし、firstDirの成分がy以外ゼロならworkUpの値を変更する(軸が求められないから)
						workUp(1, 0, 0);
					}
					auto right = cross(workUp, baseDir);
					auto axis = normalize(cross(baseDir, right));
					result = makeRotaQuaternion(axis, PI);
				}
				return result;
			} else {
				float radian = acos(d);
				auto axis = normalize(cross(baseDir, targetDir));
				return makeRotaQuaternion(axis, radian);
			}
		}

		/// @brief 位置クォータニオンを生成する
		/// @param[in] pos
		/// @retval quaternion
		inline quaternion makePosQuaternion(const float3& pos)noexcept
		{
			return quaternion(pos, 0.f);
		}

		/// @brief 共役クォータニオンを生成する
		/// @param[in] q
		/// @retval quaternion
		inline quaternion conjugate(const quaternion& q)noexcept
		{
			return quaternion(-q.x, -q.y, -q.z, q.w);
		}

		/// @brief 逆クォータニオンを生成する
		/// @param[in] q
		/// @retval quaternion
		inline quaternion inverse(const quaternion& q)noexcept
		{
			auto lenSq = lengthSq(q);
			return conjugate(q) / lenSq;
		}

		/// @brief 球面線形補間
		/// @param[in] base
		/// @param[in] target
		/// @param[in] t
		/// @retval quaternion
		inline quaternion slerp(const quaternion& base, const quaternion& target, float t)noexcept
		{
			//	内積
			float dot = base.x*target.x + base.y*target.y + base.z*target.z + base.w*target.w;
			float ss = 1.0f - (dot * dot);
			if (ss <= 0.0f) {
				return base;
			}

			quaternion Target = target;
			if (dot < 0.0f) {
				Target = -Target;
				dot = -dot;
			}
			//	内積値補正.
			if (dot > 1.0f) dot = 1.0f;
			else if (dot < -1.f) dot = -1.f;
			//	補間割合計算.
			float ph = acosf(dot);
			float s = 1 / sinf(ph);
			float s1 = sinf(ph * (1.0f - t)) * s;
			float s2 = sinf(ph *  t) * s;
			//	補間.
			return base*s1 + Target*s2;
		}

		/// @brief クォータニオンによる3次元の位置の回転
		/// @param[in] pos
		/// @param[in] q
		/// @retval float3
		inline float3 rotatePos(const float3& pos, const quaternion& q)noexcept
		{
			float x = q.y * pos.z - q.z * pos.y + q.w * pos.x;
			float y = q.z * pos.x - q.x * pos.z + q.w * pos.y;
			float z = q.x * pos.y - q.y * pos.x + q.w * pos.z;
			float w = -q.x * pos.x - q.y * pos.y - q.z * pos.z;

			float3 result;
			result.x = -y * q.z + z * q.y + x * q.w - w * q.x;
			result.y = -z * q.x + x * q.z + y * q.w - w * q.y;
			result.z = -x * q.y + y * q.x + z * q.w - w * q.z;
			return result;
		}

		/// @brief クォータニオンの合成
		/// 回転の掛け順は右手系になっているので注意してください
		/// @param[in] a
		/// @param[in] b
		/// @retval quaternion
		inline quaternion mulQ(const quaternion& a, const quaternion& b)noexcept
		{
			return quaternion(
				a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
				a.y * b.z - a.z * b.y + a.x * b.w + a.w * b.x,
				a.z * b.x - a.x * b.z + a.y * b.w + a.w * b.y,
				a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z
				);
		}

		/// @brief 回転クォータニオンから行列に変換する
		/// @tparam T
		/// @param[in] q
		/// @retval float4x4
		template<typename T>
		inline T convertRota(const quaternion& q)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::convertRota : T don't have math::buffer...");
			static_assert(T::ROW == T::COL, "ERROR math::convertRota : T isn't square matrix...");
			static_assert(3 <= T::ROW && T::ROW <= 4, "ERROR math::convertRota : T row isn't 3 or 4...");
			static_assert(3 <= T::COL && T::COL <= 4, "ERROR math::convertRota : T col isn't 3 or 4...");
			T result;
			setIdentity(result);

			float s = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
			s = 2.0f / (s * s);
			float vx = q.x * s;
			float vy = q.y * s;
			float vz = q.z * s;
			float wx = vx * q.w;
			float wy = vy * q.w;
			float wz = vz * q.w;
			float sx = q.x * vx;
			float sy = q.y * vy;
			float sz = q.z * vz;
			float cx = q.y * vz;
			float cy = q.z * vx;
			float cz = q.x * vy;

			result._11 = 1.0f - sy - sz;
			result._12 = cz + wz;
			result._13 = cy - wy;
			result._14 = 0.0f;
			result._21 = cz - wz;
			result._22 = 1.0f - sx - sz;
			result._23 = cx + wx;
			result._24 = 0.0f;
			result._31 = cy + wy;
			result._32 = cx - wx;

			return result;
		}

		/// @brief 行列から回転クォータニオンに変換する
		/// @tparam T
		/// @param[in] mat
		/// @retval quaternion
		template<typename T>
		inline quaternion convertRota(const T& mat)noexcept
		{
			static_assert(meta::_is_has_buffer<T>::value, "ERROR math::convertRota : T don't have math::buffer...");
			static_assert(T::ROW == T::COL, "ERROR math::convertRota : T isn't square matrix...");
			static_assert(3 <= T::ROW && T::ROW <= 4, "ERROR math::convertRota : T row isn't 3 or 4...");
			static_assert(3 <= T::COL && T::COL <= 4, "ERROR math::convertRota : T col isn't 3 or 4...");

			float len[4] = {
				(mat._11 - mat._22 - mat._33 + 1.f),
				(-mat._11 + mat._22 - mat._33 + 1.f),
				(-mat._11 - mat._22 + mat._33 + 1.f),
				(mat._11 + mat._22 + mat._33 + 1.f),
			};
			//一番大きい成分を求める.
			int biggestIndex = 0;
			for (int i = 1; i<4; i++) {
				if (len[biggestIndex] < len[i]) {
					biggestIndex = i;
				}
			}
			//エラーチェック.
			if (len[biggestIndex] < 0.f) {
				return quaternion(0, 0, 0, 1);
			}
			quaternion result;
			float *q[4] = { &result.x, &result.y, &result.z, &result.w };
			*q[biggestIndex] = sqrt(len[biggestIndex]) * 0.5f;
			float mult = 0.25f / *q[biggestIndex];
			switch (biggestIndex) {
			case 0: // x成分が1番大きかった場合.
				*q[1] = (mat._12 + mat._21) * mult;
				*q[2] = (mat._31 + mat._13) * mult;
				*q[3] = (mat._23 - mat._32) * mult;
				break;
			case 1: // y成分が1番大きかった場合.
				*q[0] = (mat._12 + mat._21) * mult;
				*q[2] = (mat._23 + mat._32) * mult;
				*q[3] = (mat._31 - mat._13) * mult;
				break;
			case 2: // z成分が1番大きかった場合.
				*q[0] = (mat._31 + mat._13) * mult;
				*q[1] = (mat._23 + mat._32) * mult;
				*q[3] = (mat._12 - mat._21) * mult;
				break;
			case 3: // w成分が1番大きかった場合.
				*q[0] = (mat._23 - mat._32) * mult;
				*q[1] = (mat._31 - mat._13) * mult;
				*q[2] = (mat._12 - mat._21) * mult;
				break;
			}
			return result;
		}
	}
}
