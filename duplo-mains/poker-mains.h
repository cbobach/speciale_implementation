#include "ezOptionParser/ezOptionParser.hpp"
#include <cryptoTools/cryptoTools/Common/BitVector.h>
#include <util/util.h>
#include <random>

using namespace ez;

//Hardcoded default values
static std::string default_circuit_file("circuits/cb/conditional_swap_shuffle.wir.GC_duplo");
static std::string default_num_iters("1");
static std::string default_interactive("1");
static std::string default_execs("1,1,1");
static std::string default_ip_address("127.0.0.1");
static std::string default_port_const("28001");

void Usage(ezOptionParser& opt) {
  std::string usage;
  opt.getUsage(usage);
  std::cout << usage;
};


static osuCrypto::u64 DECK_SIZE = 52;
static osuCrypto::u64 CARD_SIZE = 6;
static osuCrypto::u64 HAND_SIZE = 5;

static osuCrypto::u64 SIZE_OUTPUT = DECK_SIZE * CARD_SIZE;

static int const_first_card_index = 0;
static int const_first_change_card_index = 5;
static int eval_first_card_index = 10;
static int eval_first_change_card_index = 15;

void PrintBinSep(uint8_t value[], int num, int sep) {
  for (int i = 0; i < num * sep; ++i) {
    if (i != 0 && i % sep == 0) {
      std::cout << " ";
    }
    std::cout << (unsigned int) GetBit(i, value);
  }
  std::cout << std::endl;
}

void PrintVector(osuCrypto::BitVector data, int sep) {
  for (int k = 0; k < data.size(); ++k) {
    if (k != 0 && k % sep == 0) {
      std::cout << " ";
    }
    std::cout << (int) GetBit(k, data.data());
  }

  std::cout << std::endl;
}

void PrintHand(uint8_t hand[]) {
  std::cout << "CARD#:\t";

  for (int j = 0; j < HAND_SIZE; ++j) {
    std::cout << j + 1 << ":\t";
  }

  std::cout << std::endl << "\t";

  for (int k = 0; k < HAND_SIZE; ++k) {
    int card = hand[k];

    int c = card % 4;
    std::string colour = "";
    switch (c) {
      case 0:
        colour = "\u2660 ";
        break;
      case 1:
        colour = "\u2665 ";
        break;
      case 2:
        colour = "\u2666 ";
        break;
      case 3:
        colour = "\u2663 ";
        break;
    }

    int v = card % 13;
    std::string value = "";
    switch (v) {
      case 1:
        value = "A";
        break;
      case 11:
        value = "J";
        break;
      case 12:
        value = "Q";
        break;
      case 0:
        value = "K";
        break;
      default:
        value = std::to_string(v);
    }

    std::cout << colour << value << "\t";
  }

  std::cout << std::endl;
  std::cout << "=================================================" << std::endl << std::endl;
}

std::vector<uint8_t> GetCardsToChange() {
  std::cout << "CHOSE WHICH CARDS TO CHANGE BY SELECTING THE CARD NUMBER SEPERATED BY `,` AS:"
            << std::endl
            << "\t3 to change the third card" << std::endl
            << "\t1,2 to change the first two cards" << std::endl
            << "\t0 if no card need changed" << std::endl
            << std::endl
            << "HIT ENTER TO CONFIRM SELECTION" << std::endl;

  std::string input;
  std::cin >> input;

  int pos = 0;
  std::string s;
  std::vector<uint8_t> res;
  while ((pos = input.find(",")) != std::string::npos) {
    s = input.substr(0,pos);
    res.emplace_back((uint8_t) std::stoi(s));
    input.erase(0, pos + 1);
  }

  res.emplace_back((uint8_t) std::stoi(input));

  if (res.size() > 5) {
    std::cout << "=================================================" << std::endl << std::endl
              << "TO MANY CARDS HAS BEEN CHOSEN" << std::endl
              << std::endl
              << "TRY AGAIN" << std::endl;

    res = GetCardsToChange();
  }

  if (res[0] == 0){
    res.resize(0);
  }

  return res;
}

osuCrypto::BitVector Convert6bitTo8BitVector(osuCrypto::BitVector vector) {
  osuCrypto::BitVector tmp, dzv("00");
  tmp.copy(vector, 0, vector.size());

  vector.resize(0);
  for (int j = 0; j < tmp.size(); ++j) {
    if (j % 6 == 5)
      vector.append(dzv.data(), 2, 0);

    vector.append(tmp.data(), 1, j);
  }

  return vector;
}

osuCrypto::BitVector GetSeed(double size) {
  osuCrypto::PRNG rnd;
  std::random_device engine;

  uint8_t s[16];
  for (int k = 0; k < 16; ++k) {
    s[k] = (uint8_t) engine();
//    s[k] = 0;
  }

  rnd.SetSeed(load_block(s));

  int array_size = size/8;
  uint8_t seed[array_size];
  rnd.get<uint8_t>(seed, (osuCrypto::u64) array_size);

  osuCrypto::BitVector vector(seed, size);
  return vector;
}

std::vector<uint32_t> GetFirstHandIndices(int start_index, int num_cards) {
  std::vector<uint32_t> res;

  start_index *= CARD_SIZE;
  for (int j = 0; j < num_cards * CARD_SIZE; ++j) {
    res.emplace_back(start_index);
    ++start_index;
  }

  return res;
}

std::vector<uint32_t> GetFinalHandIndices(int num_cards_changed, std::vector<uint32_t> first_indicies, uint8_t changed[], int next_out_card) {
  std::vector<uint32_t> res = first_indicies;

  next_out_card *= CARD_SIZE;
  int start_index = 0;
  for (int j = 0; j < num_cards_changed ; ++j) {
    start_index = (changed[j] - 1) * CARD_SIZE;

    for (int k = 0; k < CARD_SIZE; ++k) {
      res[start_index] = next_out_card;

      ++start_index;
      ++next_out_card;
    }
  }

  return res;
}