#include "ezOptionParser/ezOptionParser.hpp"
#include <cryptoTools/cryptoTools/Common/BitVector.h>
#include <util/util.h>

using namespace ez;

//Hardcoded default values
static std::string default_num_iters("10");
static std::string default_execs("1,1,1");
static std::string default_ip_address("localhost");
static std::string default_port_const("28001");
static std::string default_port_const_p1("28002");
static std::string default_port_const_p2("28003");
static std::string default_port_eval_p1("28004");
static std::string default_port_eval_p2("28005");
static std::string default_ram_only("0");

void Usage(ezOptionParser& opt) {
  std::string usage;
  opt.getUsage(usage);
  std::cout << usage;
};

static osuCrypto::u64 DECK_SIZE = 52;
static osuCrypto::u64 CARD_SIZE = 6;
static osuCrypto::u64 HAND_SIZE = 5;

static osuCrypto::u64 SIZE_OUTPUT = DECK_SIZE * CARD_SIZE;

static osuCrypto::u64 SIZE_SEED_SECRET_SHARE = DECK_SIZE * CARD_SIZE;
static osuCrypto::u64 SIZE_SEED_SHUFFLE = 830;
static osuCrypto::u64 SIZE_SEED_TAG = 0;
static osuCrypto::u64 SIZE_SEED = SIZE_SEED_SHUFFLE + SIZE_SEED_SECRET_SHARE + SIZE_SEED_TAG;

void PrintBinSep(uint8_t value[], int num, int sep) {
  for (int i = 0; i < num * sep; ++i) {
    if (i != 0 && i % sep == 0) {
      std::cout << " ";
    }
    std::cout << (unsigned int) GetBit(i, value);
  }
  std::cout << std::endl;
}

void PrintHand(uint8_t hand[]) {
  for (int k = 0; k < sizeof(hand); ++k) {
    uint8_t card = hand[k];
    uint8_t c = (uint8_t) (card % 4);
    std::string color = "";

    switch (c) {
      case 0: color = "\u2660 ";
      case 1: color = "\u2661 ";
      case 2: color = "\u2662 ";
      case 3: color = "\u2663 ";
    }

    std::string value = std::to_string(card % 13);

    std::cout << k << ": " << color << value << std::endl;
  }
}