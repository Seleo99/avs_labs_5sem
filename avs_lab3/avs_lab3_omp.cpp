#include <iostream>
#include <omp.h>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iomanip>

//вариант 12 - свертка двух последовательностей
//Через преобразования Фурье можно за 3*n*logn + n накодить))))

template<typename T>
void print(std::vector<T> const &vc){
    std::cout.setf(std::ios::fixed);
    std::cout.precision(3);      // до 6 включит
    for (T const &el: vc){
        std::cout<<std::setw(25)<<std::left<<el<<" ";
    }
    std::cout<<std::endl;
}

template<typename T>
std::vector<T> randomVector(size_t const &size)
{
    std::vector<T> v(size);
    std::random_device r;
    generate(v.begin(), v.end(), [&]{return r();});
    return v;
}

template<typename T>
const std::vector<T> convolution(std::vector<T> const &seq1, const size_t size_seq1, std::vector<T> const & seq2, const size_t size_seq2) {
    std::vector<T> result(size_seq1 + size_seq2 - 1, 0);
    for (size_t i = 0; i < result.size(); ++i) {
        for (size_t j = 0; j < seq2.size(); ++j) {
            if (i - j < seq1.size()) {
                result[i] += seq1[i - j] * seq2[j];
            }
        }
    }
    return result;
}

int main() {
    const size_t size_seq1 = 3;
    const size_t size_seq2 = 2;
    //std::vector<double> seq1(randomVector<double>(size_seq1)), seq2(randomVector<double>(size_seq2));
    std::vector<double> seq1{2, -2, 1};
    std::vector<double> seq2{1, 2};
    //дополняем нулями
    std::vector<double> zeros1(size_seq2 - 1, 0), zeros2(size_seq1 - 1, 0);
    seq1.insert(seq1.end(), std::make_move_iterator(zeros1.begin()), std::make_move_iterator(zeros1.end()));
    seq2.insert(seq2.end(), std::make_move_iterator(zeros2.begin()), std::make_move_iterator(zeros2.end()));
    std::cout << "Seq1: ";
    print(seq1);
    std::cout << "Seq2: ";
    print(seq2);
    std::vector<double> res = convolution(seq1, size_seq1, seq2, size_seq2);
    std::cout<<"Resl: ";
    print(res);
    return 0;
}


/*#pragma omp parallel for default(shared) private(i, j)
    /*for (i = 0; i < N; ++i) {

    }
*/

//pragma openMP parallel for
//для POSIX-систем:
//1. Определено количество ядер
//код под прагмой будет вынесен в отдельную функцию
//создается столько posix-threads, сколько ядер на этой системе
//каждому потоку на исполнение передастся сгенерированная функция
//на месте } сгенерится join по всем потокам
//каждому потоку передано то пространство итерирования, которое для него специфично

//pragma omp for
//pragma omp sections
//pragma omp single

//private
//pragma omp task