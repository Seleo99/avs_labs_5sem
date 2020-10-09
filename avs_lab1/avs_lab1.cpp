#include <iostream>
#include <mmintrin.h> //pentium mmx
//<x86intrin.h> since gcc-4.5.0

//12 variant
// F[i]=(A[i] - B[i]) * ( C[i] + D[i] ), i=1...8;
//A, B и С – 8 разрядные целые знаковые числа (_int8);
//D – 16 разрядные целые знаковые числа (_int16).
#pragma GCC optimize("Ofast")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")
#include <iostream>
#include <immintrin.h>
#include <xmmintrin.h>
#include <memory.h>
#include <assert.h>

const size_t arr_size = 8;

template<class T, size_t N>
void read_array(T (&arr)[N]){

    if (typeid(T) == typeid(int8_t)){
        for (int i = 0; i < N; ++i) {
            std::cin >> arr[i];
            arr[i] -= '0'; //sad story
        }
    }
    else{
        for (int i = 0; i < N; ++i) {
            std::cin >> arr[i];
        }
    }
}

union U128s{
    __m64 v[2];
    int16_t array_int16[arr_size]; // 16*arr_size = 128 = 64 * 2
};

union U64s{
    __m64 v;
    int8_t array_int8[arr_size]; // 8*arr_size = 64
};

void read_data(const char * filename, U64s &a, U64s &b, U64s &c, U128s &d){
    freopen(filename, "r", stdin);
    read_array(a.array_int8);
    read_array(b.array_int8);
    read_array(c.array_int8);
    read_array(d.array_int16);
}

template<class T, size_t N>
void print_array(T (&arr)[N]){
    for (int i = 0; i < N; ++i) {
        if (typeid(arr[i]) == typeid(int8_t))
            std::cout << static_cast<int16_t>(arr[i]) << ' ';
        else
            std::cout << arr[i] << ' ';
    }
    std::cout << std::endl;
}

U128s U64s_to_U128s(const U64s&  from){
    U128s to;
    for (int i = 0; i < arr_size; ++i) {
        to.array_int16[i] = static_cast<int16_t>(from.array_int8[i]);
    }
    to.v[0] = _mm_setr_pi16(to.array_int16[0], to.array_int16[1], to.array_int16[2] , to.array_int16[3]);
    to.v[1] = _mm_setr_pi16(to.array_int16[4], to.array_int16[5], to.array_int16[6] , to.array_int16[7]);
    return to;
} //checked

std::string test_data(U64s const &a, U64s const &b, U64s const &c, U128s const &d, U128s const &result){
    std::string ret_value;
    for (int i = 0; i < arr_size; ++i){
        assert((a.array_int8[i] - b.array_int8[i])*(c.array_int8[i] + d.array_int16[i]) == result.array_int16[i]);
    }
    return "ok";
}

U128s calc_result(U64s &a, U64s &b, U64s &c, U128s &d){
    U64s sub;
    U128s summ, multik;
    sub.v = _mm_sub_pi8(a.v, b.v);
    summ.v[0] = _mm_add_pi16(d.v[0], U64s_to_U128s(c).v[0]);
    summ.v[1] = _mm_add_pi16(d.v[1], U64s_to_U128s(c).v[1]);
    multik.v[0] = _mm_mullo_pi16(summ.v[0], U64s_to_U128s(sub).v[0]);
    multik.v[1] = _mm_mullo_pi16(summ.v[1], U64s_to_U128s(sub).v[1]);
    return multik;
}

int main() {
    U64s a, b, c;
    U128s d, multik;
    const char* filename1 = "../input2.txt"; //input2
    const char* filename2 = "../input.txt";
    //(A[i] - B[i]) * ( C[i] + D[i] )
    read_data(filename1, a, b, c, d);
    multik = calc_result(a, b, c, d);
    std::cout<<test_data(a, b, c, d, multik)<<std::endl;

    read_data(filename2, a, b, c, d);
    multik = calc_result(a, b, c, d);
    std::cout<<test_data(a, b, c, d, multik)<<std::endl;
    return 0;
}
