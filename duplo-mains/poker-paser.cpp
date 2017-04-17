#include <iostream>
#include <bitset>
#include <vector>


#include "test.h"
#include "AES_Encrypt.h"

#include "util/util.h"

std::__cxx11::string s0 = std::__cxx11::string("") +
                          "000000" + "000000" + "000000" + "000000" + "000000" +
                          "000000" + "000000" + "000000" + "000000" + "000000" +
                          "000000" + "000000" + "000000" + "000000" + "000000" +
                          "000000" + "000000" + "000000" + "000000" + "000000";

std::__cxx11::string s0_19 = std::__cxx11::string("") +
                          "000000" + "100000" + "010000" + "110000" + "001000" + 
                          "101000" + "011000" + "111000" + "000100" + "100100" +
                          "010100" + "110100" + "001100" + "101100" + "011100" +
                          "111100" + "000010" + "100010" + "010010" + "110010";

std::__cxx11::string s20_39 = std::__cxx11::string("") +
                          "001010" + "101010" + "011010" + "111010" + "000110" + 
                          "100110" + "010110" + "110110" + "001110" + "101110" +
                          "011110" + "111110" + "000001" + "100001" + "010001" +
                          "110001" + "001001" + "101001" + "011001" + "111001";

std::__cxx11::string s40_52 = std::__cxx11::string("") +
                          "000101" + "100101" + "010101" + "110101" + "001101" +
                          "101101" + "011101" + "111101" + "000011" + "100011" +
                          "010011" + "110011" + "111111" + "111111" + "111111" +
                          "111111" + "111111" + "111111" + "111111" + "111111";

std::__cxx11::string s1 = std::__cxx11::string("") +
                          "111111" + "111111" + "111111" + "111111" + "111111" +
                          "111111" + "111111" + "111111" + "111111" + "111111" +
                          "111111" + "111111" + "111111" + "111111" + "111111" +
                          "111111" + "111111" + "111111" + "111111" + "111111";


osuCrypto::BitVector input0__0(s0 + s0);
osuCrypto::BitVector input0_19__0(s0_19 + s0);
osuCrypto::BitVector input20_39__0(s20_39 + s0);
osuCrypto::BitVector input40_52__0(s40_52 + s0);
osuCrypto::BitVector input1__0(s1 + s0);
osuCrypto::BitVector input0__1(s0 + s1);
osuCrypto::BitVector input0_19__1(s0_19 + s1);
osuCrypto::BitVector input20_39__1(s20_39 + s1);
osuCrypto::BitVector input40_52__1(s40_52 + s1);
osuCrypto::BitVector input1__1(s1 + s1);


std::__cxx11::string sl0 = std::__cxx11::string("") +
                          "000000000000000000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000000000000000000" +
                          "0000000000000000000000000000000000000000000000000" +
                          "000000000000000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000000000000000" +
                          "0000000000000000000000000000000000000000000000" +
                          "000000000000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000000000000" +
                          "0000000000000000000000000000000000000000000" +
                          "000000000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000000000" +
                          "0000000000000000000000000000000000000000" +
                          "000000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000000" +
                          "0000000000000000000000000000000000000" +
                          "000000000000000000000000000000000000" +
                          "00000000000000000000000000000000000" +
                          "0000000000000000000000000000000000" +
                          "000000000000000000000000000000000" +
                          "00000000000000000000000000000000";

std::__cxx11::string sl10 = std::__cxx11::string("") +
                           "1000000000000000000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000000000000000000" +
                           "0000000000000000000000000000000000000000000000000" +
                           "000000000000000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000000000000000" +
                           "0000000000000000000000000000000000000000000000" +
                           "000000000000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000000000000" +
                           "0000000000000000000000000000000000000000000" +
                           "000000000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000000000" +
                           "0000000000000000000000000000000000000000" +
                           "000000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000000" +
                           "0000000000000000000000000000000000000" +
                           "000000000000000000000000000000000000" +
                           "00000000000000000000000000000000000" +
                           "0000000000000000000000000000000000" +
                           "000000000000000000000000000000000" +
                           "00000000000000000000000000000000";

std::__cxx11::string sl1 = std::__cxx11::string("") +
                           "111111111111111111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111111111111111111" +
                           "1111111111111111111111111111111111111111111111111" +
                           "111111111111111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111111111111111" +
                           "1111111111111111111111111111111111111111111111" +
                           "111111111111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111111111111" +
                           "1111111111111111111111111111111111111111111" +
                           "111111111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111111111" +
                           "1111111111111111111111111111111111111111" +
                           "111111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111111" +
                           "1111111111111111111111111111111111111" +
                           "111111111111111111111111111111111111" +
                           "11111111111111111111111111111111111" +
                           "1111111111111111111111111111111111" +
                           "111111111111111111111111111111111" +
                           "11111111111111111111111111111111";

osuCrypto::BitVector inputLong0__0(sl0 + sl0);
osuCrypto::BitVector inputLong10__0(sl10 + sl0);
osuCrypto::BitVector inputLong1__0(sl1 + sl0);
osuCrypto::BitVector inputLong0__1(sl0 + sl1);
osuCrypto::BitVector inputLong1__1(sl1 + sl1);


void PrintBinSep(uint8_t value[], int num, int sep) {
    for (int i = 0; i < num * sep; ++i) {
        if (i != 0 && i % sep == 0) {
            std::cout << " ";
        }
        std::cout << (unsigned int) GetBit(i, value);
    }
    std::cout << std::endl;
}


void assertThat(ComposedCircuit frigate_circuit, osuCrypto::BitVector input, int blocks, int blockSize) {
    osuCrypto::BitVector frigate_output = eval_circuit(frigate_circuit, input);

    PrintBinSep(frigate_output.data(), blocks, blockSize);
}


void testAllInputs(const char* composed_circuit, int blocks, int blockSize) {
    ComposedCircuit frigate_circuit = read_composed_circuit(composed_circuit);

    std::cout << "\tInput: 0's 0's\t\t";
    assertThat(frigate_circuit, input0__0, blocks, blockSize);

    std::cout << "\tInput: 0-19 0's\t\t";
    assertThat(frigate_circuit, input0_19__0, blocks, blockSize);

    std::cout << "\tInput: 20-39 0's\t";
    assertThat(frigate_circuit, input20_39__0, blocks, blockSize);

    std::cout << "\tInput: 40-52 0's\t";
    assertThat(frigate_circuit, input40_52__0, blocks, blockSize);

    std::cout << "\tInput: 1's 0's\t\t";
    assertThat(frigate_circuit, input1__0, blocks, blockSize);

    std::cout << "\tInput: 0's 1's\t\t";
    assertThat(frigate_circuit, input0__1, blocks, blockSize);

    std::cout << "\tInput: 0-19 1's\t\t";
    assertThat(frigate_circuit, input0_19__1, blocks, blockSize);

    std::cout << "\tInput: 20-39 1's\t";
    assertThat(frigate_circuit, input20_39__1, blocks, blockSize);

    std::cout << "\tInput: 40-52 1's\t";
    assertThat(frigate_circuit, input40_52__1, blocks, blockSize);

    std::cout << "\tInput: 1's 1's\t\t";
    assertThat(frigate_circuit, input1__1, blocks, blockSize);

    std::cout << std::endl;
}


void testAllLongInputs(const char* composed_circuit, int blocks, int blockSize) {
    ComposedCircuit frigate_circuit = read_composed_circuit(composed_circuit);

    std::cout << "\tInput: 0's 0's\t\t";
    assertThat(frigate_circuit, inputLong0__0, blocks, blockSize);

    std::cout << "\tInput: 10's 0's\t\t";
    assertThat(frigate_circuit, inputLong10__0, blocks, blockSize);

    std::cout << "\tInput: 1's 0's\t\t";
    assertThat(frigate_circuit, inputLong1__0, blocks, blockSize);

    std::cout << "\tInput: 0's 1's\t\t";
    assertThat(frigate_circuit, inputLong0__1, blocks, blockSize);

    std::cout << "\tInput: 1's 1's\t\t";
    assertThat(frigate_circuit, inputLong1__1, blocks, blockSize);

    std::cout << std::endl;
}

int main(int argc, const char* argv[]) {
    /*
     * SEED 1
     * */
    std::cout << "INPUT PARTY 1:" << std::endl;
    testAllInputs("./circuits/cb/test_input_party1.wir.GC_duplo", 20, 6);
    
    
    /*
     * SEED 2
     * */
    std::cout << "INPUT PARTY 2:" << std::endl;
    testAllInputs("./circuits/cb/test_input_party2.wir.GC_duplo", 20, 6);


    /*
     * ADD
     * */
//    std::cout << "ADD INPUTS:" << std::endl;
//    testAllInputs("./circuits/cb/test_add.wir.GC_duplo", 20, 6);


    /*
     * MOD
     * */
//    std::cout << "MOD:" << std::endl;
//    testAllInputs("./circuits/cb/test_mod.wir.GC_duplo", 20, 6);


    /*
     * CORRECT SEED
     * */
    std::cout << "CORRECT SEEDS:" << std::endl;
    testAllInputs("./circuits/cb/test_corrected_seed.wir.GC_duplo", 20, 6);


    /*
     * INIT DECK
     * */
//    std::cout << "INIT DECK:" << std::endl;
//    testAllInputs("./circuits/cb/test_init_deck.wir.GC_duplo", 52, 6);


    /*
     * FISHER-YATES
     * */
    std::cout << "FISHER-YATES:" << std::endl;
    testAllInputs("./circuits/cb/fisher_yates_shuffle.wir.GC_duplo", 20, 6);


    /*
     * XOR SEED
    * */
    std::cout << "XOR SEEDS:" << std::endl;
    testAllLongInputs("./circuits/cb/test_xor.wir.GC_duplo", 1, 830);


    /*
     * CONDITIONAL SWA{
    * */
    std::cout << "CONDITIONAL SWAP:" << std::endl;
    testAllLongInputs("./circuits/cb/conditional_swap_shuffle.wir.GC_duplo", 52, 6);
}