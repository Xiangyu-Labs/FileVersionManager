/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef ENCRYPTOR_CPP
#define ENCRYPTOR_CPP

#include <cstring>
#include <cmath>
#include <vector>

/**
 * @brief 
 * This structure is used to store complex numbers and overloads the addition, 
 * subtraction, and multiplication operators of complex numbers.
 * Among them, a holds the real part and b holds the imaginary part.
 */
struct Complex {
    double a, b;

    Complex();
    Complex(double a, double b);

    Complex operator+(const Complex &r) const;
    Complex operator-(const Complex &r) const;
    Complex operator*(const Complex &r) const;
};

/**
 * @brief 
 * This class mainly completes the encryption and decryption of a given integer sequence.
 * The encryption algorithm is FFT. It converts the polynomial function corresponding to 
 * the integer sequence from the coefficient representation to the point value 
 * representation. 
 * This process is also called discrete Fourier transform, and we accelerate this process
 * through FFT.
 */
class Encryptor {

public:
/**
 * @brief 
 * The Fast Fourier Transform requires that the sequence to be transformed must be 
 * an integer power of 2, so 2 to the power of 10 is used here.
 */
static const int N = 1 << 10;

private:
    /**
     * @brief 
     * Because it is impossible for all integer sequences to be exactly an integer 
     * power of 2, here the integer part that is less than 2^10 is added to PLACEHOLDER.
     * For example, if there is an integer sequence that is only 1000 bits long, but 
     * the FFT requires 1024 bits, 24 PLACEHOLEDER is added after this integer sequence.
     */
    static const char PLACEHOLDER = '\0';

    /**
     * @brief 
     * The buf array is used as a buffer for the FFT function.
     * Here you can use the butterfly transform to optimize this array, but the level 
     * is limited, I will not. QAQ
     * 
     * This block array is suitable for storing the data you want to encrypt.
     * An encryption sequence may be very long, but here it will be split into small 
     * data blocks, each of which has a length of N, and then this data block is spliced 
     * together to form the encrypted sequence.
     */
    Complex buf[N << 1], block[N];

    /**
     * @brief 
     * This function performs discrete Fourier transform on the sequence in array a.
     * The resulting formula is given here:
     *  A(\omega_{n}^{k+\frac{n}{2}})=A_{1}(\omega_{n}^{2k+n})+\omega_{n}^{k+\frac{n}{2}}\cdot A_{2}(\omega_{n}^{2k+n})
     * You can view the formula through latex or katex.
     * 
     * @param a 
     * The sequence you want to encrypt is stored in this array.
     * 
     * @param n 
     * The current length to be encrypted. 
     * (Of course I know that the initial length is N, but this function needs to be 
     * processed recursively, so this length will continue to be halved until it is 
     * reduced to 1, and then start to backtrack.)
     * 
     * @param type 
     * The value here can only be 1 or -1.
     * 1 represents that the sequence needs to be forward transformed by discrete 
     * Fourier transform, and -1 represents that the sequence needs to be transformed 
     * into inverse discrete Fourier transform.
     * Please refer to the document for specific formula derivation.
     */
    void fft(Complex a[], int n, int type);

    /**
     * @brief 
     * Encrypt the sequence stored in the block.
     * 
     * @param res 
     * Save the encrypted sequence in this array.
     * 
     * @return true 
     * Encryption is successful. .
     * 
     * @return false 
     * It is impossible to fail, at least for now... QAQ
     */
    bool encrypt_block(std::vector<std::pair<double, double>> &res);

    /**
     * @brief 
     * Decrypt the sequence stored in the block.
     * 
     * @param res 
     * The decrypted sequence will be stored in this array.
     * 
     * @return true 
     * The decryption is successful.
     * 
     * @return false 
     * Just like encryption, this seems impossible to fail, but it is possible that 
     * the decrypted sequence has no meaning, but it is not what this function does 
     * to check whether it is meaningful.
     */
    bool decrypt_block(std::vector<int> &res);

protected:

    /**
     * @brief 
     * This function encrypts the given sequence of integers.
     * This function will split the entire integer sequence into a data block of length N. 
     * For data blocks of length less than N, PLACEHOLDER will be added to the end until 
     * the length is N.
     * You don't need to worry about adding PLACEHOLDER here, there will be some strange 
     * things when decrypting. Here after segmentation, a bit will be added to the sequence 
     * to record the length of the sequence. When decrypting, the original sequence will 
     * be restored according to the length of the sequence, and no additional useless data 
     * will appear.
     * 
     * @param sequence 
     * The sequence of integers you want to encrypt.
     * 
     * @param res 
     * The encrypted sequence is stored in this array.
     * 
     * @return true 
     * This data was successfully encrypted.
     * 
     * @return false 
     * Impossible to fail hahaha.
     */
    bool encrypt_sequence(std::vector<int> &sequence, std::vector<std::pair<double, double>> &res);

    /**
     * @brief 
     * This function will decrypt the encrypted sequence you have given.
     * 
     * @param sequence 
     * The sequence you want to decrypt.
     * 
     * @param res 
     * The decrypted data is stored in this array.
     * 
     * @return true 
     * The data was successfully decrypted.
     * 
     * @return false 
     * If the length of the encrypted sequence is not a multiple of N, then this function 
     * will return an error.
     * If an error is returned, you should check whether the data is damaged. 
     * Of course, there may be a problem with this saver.. The saver is too sophisticated, 
     * so that even a little problem may cause the original data to be unable to be restored 
     * intact, leading to errors.
     */
    bool decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res);
};



                        /* ======= struct Complex ======= */
Complex::Complex() = default;
Complex::Complex(double a, double b) : a(a), b(b) {}
Complex Complex::operator+(const Complex &r) const {
    return Complex(a + r.a, b + r.b);
}
Complex Complex::operator-(const Complex &r) const {
    return Complex(a - r.a, b - r.b);
}
Complex Complex::operator*(const Complex &r) const {
    return Complex(a * r.a - b * r.b, a * r.b + b * r.a);
}

                        /* ======= class Encryptor ======= */
void Encryptor::fft(Complex a[], int n, int type) {
    const static double Pi = acos(-1.0);
    if (n == 1) return;
    int m = n >> 1;
    for (int i = 0; i < m; i++) {
        buf[i] = a[i << 1];
        buf[i + m] = a[i << 1 | 1];
    }
    memcpy(a, buf, sizeof(Complex) * n);
    fft(a, m, type);
    fft(a + m, m, type);
    Complex wn = Complex(1, 0), u = Complex(cos(2 * Pi / n), type * sin(2 *Pi / n));
    for (int i = 0; i < m; i++) {
        Complex t = wn * a[m + i];
        wn = wn * u;
        buf[i] = a[i] + t;
        buf[i + m] = a[i] - t;
    }
    memcpy(a, buf, sizeof(Complex) * n);
}

bool Encryptor::encrypt_block(std::vector<std::pair<double, double>> &res) {
    fft(block, N, 1);
    res.clear();
    for (int i = 0; i < N; i++) {
        res.push_back(std::make_pair(block[i].a, block[i].b));
    }
    return true;
}

bool Encryptor::decrypt_block(std::vector<int> &res) {
    fft(block, N, -1);
    res.clear();
    for (int i = 0; i < N; i++) {
        res.push_back((int)floor(block[i].a / N + 0.5));
    }
    return true;
}

bool Encryptor::encrypt_sequence(std::vector<int> &sequence, std::vector<std::pair<double, double>> &res) {
    long long len = sequence.size();
    std::vector<int> padded = sequence;
    int idx = 1;
    while ((padded.size() + 1) % N != 0) padded.push_back(PLACEHOLDER);
    memset(block, 0, sizeof block);
    block[0].a = len;
    res.clear();
    std::vector<std::pair<double, double>> tmp;
    for (auto &it : padded) {
        block[idx++].a = it;
        if (idx == N) {
            encrypt_block(tmp);
            res.insert(res.end(), tmp.begin(), tmp.end());
            idx = 0;
            memset(block, 0, sizeof block);
            tmp.clear();
        }
    }
    return true;
}

bool Encryptor::decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res) {
    if (sequence.empty()) return false;
    if (sequence.size() % N != 0) return false;
    memset(block, 0, sizeof block);
    int idx = 0;
    res.clear();
    long long len = -1;
    std::vector<int> tmp;
    for (auto &it : sequence) {
        block[idx].a = it.first;
        block[idx++].b = it.second;
        if (idx == N) {
            decrypt_block(tmp);
            if (len != -1) {
                res.insert(res.end(), tmp.begin(), tmp.end());
            } else {
                len = tmp.front();
                res.insert(res.end(), tmp.begin() + 1, tmp.end());
            }
            idx = 0;
            memset(block, 0, sizeof block);
            tmp.clear();
        }
    }
    if (len < 0 || (unsigned long long)len > res.size()) {
        res.clear();
        return false;
    }
    res.erase(res.begin() + len, res.end());
    return true;
}

#endif
