#include "rice/Class.hpp"
#include "rice/String.hpp"
#include "rice/Array.hpp"
#include "rice/Exception.hpp"
#include "libpopcnt.h"
#include <iostream>
#include <sstream>
#include <string>
#include <bitset>

using namespace Rice;

  // This is the core of the Hamming distance for inputs of length in bytes being multiple of 8
  // buf is used the store the results of xor of the two primary inputs and the lengths are assumed to match
  inline uint64_t hamming_64(const uint64_t* a64, const uint64_t* b64,  std::vector<uint64_t>& buf) {
     for (size_t k = 0; k < buf.size(); k++) {
              buf[k] =  a64[k] ^ b64[k];
     }
     uint64_t dist =  popcnt((const void*) &buf[0], 8 * buf.size());
     return dist;
  }


  // A less efficient version where length is not multiple of 8 bytes
  inline uint64_t hamming_8(const uint8_t* a8, const uint8_t* b8,  std::vector<uint8_t>& buf) {
     for (size_t k = 0; k < buf.size(); k++) {
              buf[k] =  a8[k] ^ b8[k];
     }
     uint64_t dist =  popcnt((const void*) &buf[0], buf.size());
     return dist;
  }


  Object hamcpp_hello(Object /* self */)
  {
    String str("hello, world");
    return str;
  }


  // return the bit representation of the pack string, eg '1001010101010101'
  // size of the output string will be x 8 size of the input string 
  Object hamcpp_bitset(Object self, std::string x)
  {
    std::stringstream ss;
    for(size_t i = 0; i < x.size(); i++) {
       std::bitset<8> b(x[i]);
       ss << b;
    }
    return String(ss.str());
  }

  // check if the pair is noempty and consistently sized
  // optionally check 64-bit boundary
  inline bool is_valid_bitstring_pair(const std::string& sa, const std::string& sb, const bool check64) {
     if (sa.size() < 1 || sb.size() < 1)
       throw Exception(rb_eRuntimeError, "input strings must not be empty");
     if (sa.size() != sb.size())
       throw Exception(rb_eRuntimeError, "input strings must be of equal length");

     if (check64)
       if ((sa.size() % 8 != 0) || (sb.size() % 8 != 0))
         throw Exception(rb_eRuntimeError, "input strings must be multiple of 8 bytes long");

     return true;
  }

  // Hamming distance between two packet bit arrays
  Object hamcpp_hamming_8(Object self, std::string sa, std::string sb)
  {
     is_valid_bitstring_pair(sa, sb, false);

     std::vector<uint8_t> buf(sa.size()); // stores XOR between two signatures
     const uint8_t* a8 = (uint8_t*) &sa[0];
     const uint8_t* b8 = (uint8_t*) &sb[0];
     uint64_t dist = hamming_8(a8, b8, buf);
     return to_ruby(dist);;
  }


  // As above but with lengh multiple of 64 bits
  Object hamcpp_hamming_64(Object self, std::string sa, std::string sb)
  {
     is_valid_bitstring_pair(sa, sb, true); // also check 64-bit boundary

     std::vector<uint64_t> buf(sa.size() / 8); // stores XOR between two signatures
     const uint64_t* a64 = (uint64_t*) &sa[0]; // length in bytes should be a multiple of 8
     const uint64_t* b64 = (uint64_t*) &sb[0]; // length in bytes should be a multiple of 8
     uint64_t dist = hamming_64(a64, b64, buf);
     return to_ruby(dist);;
  }


  // print out the string bit representation and the number of bits
  Object hamcpp_str_info(Object self, std::string x)
  {
    int ones_cnt =  popcnt(&x, x.size());
    std::cout << x << "  " <<  x.size() << " pop count " << ones_cnt << std::endl;
    for(size_t i=0; i < x.size(); i++) {
       std::bitset<8> b(x[i]);
       std::cout << b;
    }
    std::cout << std::endl;
    return self;
  }

  void print_byte(const uint8_t byte) {
    std::bitset<8> x(byte);
    std::cout << x << std::endl;
  }

  void print_8_bytes(const uint64_t chunk) {
    std::bitset<64> x(chunk);
    std::cout << x << std::endl;
  }

  void print_bytes(const std::vector<uint8_t>& bytes) {
    for(size_t i=0; i < bytes.size(); i++) {
       std::bitset<8> b(bytes[i]);
       std::cout << b;
    }
    std::cout << std::endl;
  }

  void print_bytes(const std::string& bytes) {
    for(size_t i=0; i < bytes.size(); i++) {
      std::bitset<8> x((uint8_t)bytes[i]);
      std::cout << x;
    }
    std::cout << std::endl;
  }



  // Print the input array bistrings as well as their population counts
  Object hamcpp_array_str_info(Object self, Array a)
  {
    int idx = 0;
    for (Rice::Array::iterator it = a.begin(); it != a.end(); ++it, idx++) {
         const std::string x = from_ruby<std::string>(*it);
         int ones_cnt =  popcnt(&x, x.size());
         std::cout << idx << " " << x << "  " <<  x.size() << " pop count " << ones_cnt << std::endl;
         for(size_t i=0; i < x.size(); i++) {
             std::bitset<8> b(x[i]);
             std::cout << b;
         }
         std::cout << std::endl;
    }
    return self;
  }


  // Convert ruby array of bitstrings to c++ vector of strings
  // with validity checks on bitvector length
  void convert_array(const Array& a, std::vector<std::string> &sig_tab, const bool check64) {
    size_t idx = 0;
    for (Rice::Array::const_iterator it = a.begin(); it != a.end(); ++it, idx++) {
         const std::string sa = from_ruby<std::string>(*it);
         if (sa.size() <  1)
           throw Exception(rb_eRuntimeError, "input strings cannot be empty");
         if (check64 && sa.size() % 8 != 0)
           throw Exception(rb_eRuntimeError, "input strings must be multiple of 8 bytes long");
         if (idx > 0 && sa.size() != sig_tab[0].size())
            throw Exception(rb_eRuntimeError, "input strings must be of equal length");
         sig_tab.push_back(sa);
    }
  }


Object hamcpp_array_hamming(Object self, Array a, Object th) {
    uint64_t threshold = (uint64_t) from_ruby<int>(th); // of interest are items whose distance is at or below this value
    double sum_dist = 0.0;

    // First - copy the the input data to a vector of strings
    // to avoid accesiing input array in the inner loop. Also
    // check for bitstring size consistency
    std::vector<std::string> sig_tab;
    convert_array(a, sig_tab, false);

    Array result_tab; // store pairs of indices that score at or below threhsold
    std::vector<uint8_t> buf; // stores XOR between two signatures
    for (size_t a_idx = 0; a_idx < sig_tab.size();  a_idx++) {
         const std::string& sa = sig_tab[a_idx]; // length in bytes should be a multiple of 8
         if(a_idx == 0) {
             buf.resize(sa.size()); // stores XOR between two signatures
         }
         for (size_t b_idx = a_idx + 1; b_idx < sig_tab.size(); b_idx++) {
           const std::string& sb = sig_tab[b_idx];;
           for (size_t k = 0; k < sa.size(); k++) { 
              buf[k] = ((uint8_t) sa[k]) ^ ((uint8_t) sb[k]);
           }
           uint64_t dist =  popcnt((const void*) &buf[0], buf.size());
           if (dist <= threshold) {
            try {
              Array tmp;
              tmp.push((int) dist);
              tmp.push((int) a_idx);
              tmp.push((int) b_idx);
              result_tab.push(tmp);
            } catch (...) {
              throw Exception(rb_eRuntimeError, "Unable to allocate Ruby array");
            }
              //std::cerr << dist << "  " << a_idx << "  " << b_idx << std::endl;
           }
           sum_dist += dist;
         }
    }
    sum_dist /= 0.5 * a.size() * (a.size() - 1.0);
    std::cerr << sum_dist << "  mean dist for array of size " << a.size() << std::endl;
    return result_tab;
  }



  // This version chunks xor computation over 8-byte 64 bits ints, which is faster
  Object hamcpp_array_hamming_64(Object self, Array a, Object th) {
    uint64_t threshold = (uint64_t) from_ruby<int>(th); // of interest are items whose distance is at or below this value
    double sum_dist = 0.0;
    // First - copy the the input data to a vector of strings
    // to avoid accesiing input array in the inner loop. Also
    // check for bitstring size consistency
    std::vector<std::string> sig_tab;
    convert_array(a, sig_tab, true);  // also check 64 bit boundary

    Array result_tab; // store pairs of indices that score at or below threhsold
    std::vector<uint64_t> buf; // stores XOR between two signatures
    for (size_t a_idx = 0; a_idx < sig_tab.size();  a_idx++) {
         const std::string& sa = sig_tab[a_idx]; // length in bytes should be a multiple of 8
         const uint64_t* a64 = (uint64_t*) &sa[0]; // length in bytes should be a multiple of 8
         if(a_idx == 0) {
             buf.resize(sa.size() / 8); // stores XOR between two signatures
         }
         for (size_t b_idx = a_idx + 1; b_idx < sig_tab.size(); b_idx++) {
           const std::string& sb = sig_tab[b_idx];;
           const uint64_t* b64 = (uint64_t*) &sb[0]; // length in bytes should be a multiple of 8
           uint64_t dist = hamming_64(a64, b64, buf);
           if (dist <= threshold) {
              Array tmp;
              tmp.push((int) dist);
              tmp.push((int) a_idx);
              tmp.push((int) b_idx);
              result_tab.push(tmp);
           }
           sum_dist += dist;
         }
    }
    sum_dist /= 0.5 * a.size() * (a.size() - 1.0);
    std::cerr << sum_dist << "  mean dist for array of size " << a.size() << std::endl;
    return result_tab;
  }


  // Two arrays of signature are provided, corresponding to the same objects
  // If for given pair the short fingerprint distance is below specified threshold
  // the longer fingerprint distnace is calculated and compared against the respective threshold.
  // Both short and long fingerprints must be aligned along the 64 bit boundary.
  Object hamcpp_array_two_stage_hamming_64(Object self, Array a_short, Array a_long, Object th_short, Object th_long) {
    uint64_t threshold_short = (uint64_t) from_ruby<int>(th_short); // of interest are items whose distance is at or below this value
    uint64_t threshold_long = (uint64_t) from_ruby<int>(th_long); // of interest are items whose distance is at or below this value
    double sum_dist = 0.0;
    // First - copy the the input data to a vector of strings
    // to avoid accesiing input array in the inner loop. Also
    // check for bitstring size consistency
    std::vector<std::string> sig_tab_short, sig_tab_long;
    convert_array(a_short, sig_tab_short, true);  // also check 64 bit boundary
    convert_array(a_long, sig_tab_long, true);  // also check 64 bit boundary

    Array result_tab; // store pairs of indices that score at or below threhsold
    std::vector<uint64_t> buf_short; // stores XOR between two short fingeprints
    std::vector<uint64_t> buf_long;  // stores XOR between two long fingeprints
    for (size_t a_idx = 0; a_idx < sig_tab_short.size();  a_idx++) {
         const std::string& sa_short = sig_tab_short[a_idx];
         const std::string& sa_long = sig_tab_long[a_idx];
         const uint64_t* a64_short = (uint64_t*) &sa_short[0]; // length in bytes should be a multiple of 8
         const uint64_t* a64_long = (uint64_t*) &sa_long[0];
         if (a_idx == 0) {
             buf_short.resize(sa_short.size() / 8);
             buf_long.resize(sa_long.size() / 8); // stores XOR between two signatures
         }
         for (size_t b_idx = a_idx + 1; b_idx < sig_tab_short.size(); b_idx++) {
           const std::string& sb_short = sig_tab_short[b_idx];;
           const uint64_t* b64_short = (uint64_t*) &sb_short[0]; // length in bytes should be a multiple of 8
           uint64_t dist_short = hamming_64(a64_short, b64_short, buf_short);
           if (dist_short <= threshold_short) {
             const std::string& sb_long = sig_tab_long[b_idx];;
             const uint64_t* b64_long = (uint64_t*) &sb_long[0]; // length in bytes should be a multiple of 8
             uint64_t dist_long = hamming_64(a64_long, b64_long, buf_long);
             if (dist_long <= threshold_long) {
                Array tmp;
                tmp.push((int) dist_long);
                tmp.push((int) a_idx);
                tmp.push((int) b_idx);
                result_tab.push(tmp);
             }
           }
           sum_dist += dist_short;
         }
    }
    sum_dist /= 0.5 * a_short.size() * (a_short.size() - 1.0);
    std::cerr << sum_dist << "  mean dist (short) for array of size " << a_short.size() << std::endl;
    return result_tab;
  }


  extern "C"
  void Init_hamcpp()
  {
    Class rb_cHamcpp =
      define_class("Hamcpp")
      .define_method("hello", &hamcpp_hello)
      .define_method("sinfo", &hamcpp_str_info)
      .define_method("s2bitset2s", &hamcpp_bitset)
      .define_method("ainfo", &hamcpp_array_str_info)
      .define_method("hamming", &hamcpp_hamming_8)
      .define_method("hamming64", &hamcpp_hamming_64)
      .define_method("hamloop", &hamcpp_array_hamming)
      .define_method("hamloop64", &hamcpp_array_hamming_64)
      .define_method("ham2stage64", &hamcpp_array_two_stage_hamming_64);
  }

