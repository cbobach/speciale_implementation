//
// Created by cbobach on 10-04-17.
//

#include "poker-mains.h"

/*
 * 1: Generate randomness
 *
 * 2: Connect to Constructor and Evaluator and send randomness
 *
 * 3: Wait to receive secret shares from Constructor and Evaluator
 *      a: Reconstruct output
 *      b: Chose cards to be exchanged
 *      c: Reconstruct new output
 *
 * 4: Declare a winner
 *
 * */
int main(int argc, const char* argv[]) {
  ezOptionParser opt;

  opt.overview = "PokerClient Passing Parameters Guide.";
  opt.syntax = "PokerClient first second third forth fifth sixth";
  opt.example = "PokerClient -ip_const 10.11.100.216 -p_const 28001 -ip_eval localhost -p_const 28002\n\n";
  opt.footer = "ezOptionParser 0.1.4  Copyright (C) 2011 Remik Ziemlinski\nThis program is free and without warranty.\n";

  opt.add(
          "", // Default.
          0, // Required?
          0, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Display usage instructions.", // Help description.
          "-h",     // Flag token.
          "-help",  // Flag token.
          "--help", // Flag token.
          "--usage" // Flag token.
  );

  opt.add(
          default_ip_address.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "IP Address of Machine running TinyConst", // Help description.
          "-ip_const"
  );

  opt.add(
          "", // Default.
          1, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port to listen on/connect to", // Help description.
          "-p_const"
  );

  opt.add(
          default_ip_address.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "IP Address of Machine running TinyConst", // Help description.
          "-ip_eval"
  );

  opt.add(
          "", // Default.
          1, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port to listen on/connect to", // Help description.
          "-p_eval"
  );

  opt.add(
          "", // Default.
          1, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Party number", // Help description.
          "-party"
  );

  //Attempt to parse input
  opt.parse(argc, argv);

  //Check if help was requested and do some basic validation
  if (opt.isSet("-h")) {
      Usage(opt);
      return 1;
  }
  std::vector<std::string> badOptions;
  if (!opt.gotExpected(badOptions)) {
      for (int i = 0; i < badOptions.size(); ++i)
          std::cerr << "ERROR: Got unexpected number of arguments for option " << badOptions[i] << ".\n\n";
      Usage(opt);
      return 1;
  }

  //Copy inputs into the right variables
  int party_num;
  osuCrypto::u32 port_const, port_eval;
  std::string ip_address_const, ip_address_eval;

  opt.get("-ip_const")->getString(ip_address_const);
  opt.get("-ip_eval")->getString(ip_address_eval);
  opt.get("-p_const")->getInt((int &) port_const);
  opt.get("-p_eval")->getInt((int &) port_eval);
  opt.get("-party")->getInt(party_num);

/*
 * Generate Seed to be used in evaluation of circuit
 * */
  osuCrypto::PRNG rnd;
  uint8_t seed[SIZE_SEED/CARD_SIZE];
  rnd.SetSeed(load_block(seed));
  rnd.get<uint8_t>(seed, SIZE_SEED/CARD_SIZE);

  std::cout << "====== PARTY " << party_num << " CHOSE SEED: ======" << std::endl;

  PrintBinSep(seed, SIZE_SEED/CARD_SIZE, CARD_SIZE);


//  Connecting to constructor
  uint8_t snd;
  uint8_t rcv;


/*
 * Sending seed_p1 to Constructor
 * */
  osuCrypto::IOService ios;
  osuCrypto::Endpoint ep_const;
  std::string ep_const_name = "ep_const_p" + std::to_string(party_num);
  ep_const.start(ios, ip_address_const, port_const, osuCrypto::EpMode::Client, ep_const_name);

  osuCrypto::Channel chan_const;
  chan_const = ep_const.addChannel("const", "const");
  chan_const.send(seed, SIZE_SEED);


/*
 * Sending seed_p1 to Evaluator
 * */
  osuCrypto::Endpoint ep_eval;
  std::string ep_eval_name = "ep_eval_p" + std::to_string(party_num);
  ep_eval.start(ios, ip_address_eval, port_eval, osuCrypto::EpMode::Client, ep_eval_name);

  osuCrypto::Channel chan_eval;
  chan_eval = ep_eval.addChannel("eval", "eval");
  chan_eval.send(seed, SIZE_SEED);

  std::cout << "====== WAITING FOR PLAYER ======" << std::endl;


/*
 * Receiving secret share from constructor
 * */
  uint8_t ss_const_first_hand[HAND_SIZE];
  chan_const.recv(ss_const_first_hand, HAND_SIZE);


/*
 * Receiving secret share from Evaluator
 * */
  uint8_t ss_eval_first_hand[HAND_SIZE];
  chan_eval.recv(ss_eval_first_hand, HAND_SIZE);

  std::cout << "====== YOU GOT SECRET SHARE FROM CONSTRUCTOR ======" << std::endl;
  PrintBinSep(ss_const_first_hand, (int) HAND_SIZE, CARD_SIZE);
  PrintHand(ss_const_first_hand);

  std::cout << "====== YOU GOT SECRET SHARE FROM EVALUATOR ======" << std::endl;
  PrintBinSep(ss_eval_first_hand, (int) HAND_SIZE, CARD_SIZE);
  PrintHand(ss_eval_first_hand);

  std::cout << "====== YOU HAVE BEEN DEALT THESE CARDS ======" << std::endl;

  uint8_t hand[HAND_SIZE];
  for (int j = 0; j < sizeof(hand); ++j) {
    hand[j] = ss_const_first_hand[j] xor ss_eval_first_hand[j];
  }

  /*
   * TODO: Translate cards to values and colour
   * */

  PrintBinSep(hand, (int) HAND_SIZE, 6);
  PrintHand(hand);
}