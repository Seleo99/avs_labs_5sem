#include <iostream>
#include <omp.h>
#include <vector>
#include <random>
#include <iomanip>

//вариант 12 - свертка двух последовательностей
//Через преобразования Фурье можно за 3*n*logn + n накодить))))

template<typename T>
void print(std::vector<T> const &vc){
    std::cout.setf(std::ios::fixed);
    std::cout.precision(2);      //включит
    for (T const &el: vc){
        std::cout<<std::setw(10)<<std::left<<el<<" ";
    }
    std::cout<<std::endl;
}

template<typename T>
std::vector<T> randomVector(size_t const &size){
    std::vector<T> v(size);
    std::random_device r;
    std::uniform_int_distribution<int> uid(-50, 50);
    generate(v.begin(), v.end(), [&]{return uid(r);});
    return v;
}

template<typename T>
const std::vector<T> convolution_omp(std::vector<T> const &seq1, const size_t &size_seq1,
                                 std::vector<T> const &seq2, const size_t &size_seq2) {
    std::vector<T> result(size_seq1 + size_seq2 - 1, 0);
    alignas(16) size_t j = 0, i = 0;
    #pragma omp parallel for private(j, i)
    for (i = 0; i < result.size(); ++i) {
        for (j = 0; j < seq2.size(); ++j) {
            if (i - j < seq1.size()) {
                result[i] += seq1[i - j] * seq2[j];
            }
        }
    }
    return result;
}

template<typename T>
const std::vector<T> convolution(std::vector<T> const &seq1, const size_t &size_seq1,
                                 std::vector<T> const &seq2, const size_t &size_seq2) {
    std::vector<T> result(size_seq1 + size_seq2 - 1, 0);
    alignas(16) size_t j = 0, i = 0;
    for (i = 0; i < result.size(); ++i) {
        for (j = 0; j < seq2.size(); ++j) {
            if (i - j < seq1.size()) {
                result[i] += seq1[i - j] * seq2[j];
            }
        }
    }
    return result;
}

int main() {
    const size_t size_seq1 = 4096;
    const size_t size_seq2 = 4096;
    std::vector<double> seq1(randomVector<double>(size_seq1)), seq2(randomVector<double>(size_seq2));
    //std::vector<double> seq1{2, -2, 1};
    //std::vector<double> seq2{1, 2};
    //дополняем нулями
    std::vector<double> zeros1(size_seq2 - 1, 0), zeros2(size_seq1 - 1, 0);
    seq1.insert(seq1.end(), std::make_move_iterator(zeros1.begin()), std::make_move_iterator(zeros1.end()));
    seq2.insert(seq2.end(), std::make_move_iterator(zeros2.begin()), std::make_move_iterator(zeros2.end()));
    std::cout << "Seq1: ";
    print(seq1);
    std::cout << "Seq2: ";
    print(seq2);
    double time_begin, time_end;
    time_begin = omp_get_wtime();
    std::vector<double> res = convolution(seq1, size_seq1, seq2, size_seq2);
    time_end = omp_get_wtime();
    std::cout<<"Time without omp: "<<time_end - time_begin<<std::endl;
    time_begin = omp_get_wtime();
    res = convolution_omp(seq1, size_seq1, seq2, size_seq2);
    time_end = omp_get_wtime();
    std::cout<<"Time parallel: "<<time_end - time_begin<<std::endl;
    std::cout<<"Resl: ";
    print(res);
    return 0;
}

//pragma openMP parallel for
//для POSIX-систем:
//Определено количество ядер
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