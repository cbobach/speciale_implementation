#include "duplo-mains/poker-mains.h"
#include "duplo/duplo-evaluator.h"

int main(int argc, const char* argv[]) {
  ezOptionParser opt;

  opt.overview = "PokerEvaluator Passing Parameters Guide.";
  opt.syntax = "PokerEvaluator first second third forth fifth sixth";
  opt.example = "PokerEvaluator -n 4 -c aes -e 8,2,1 -ip_const 10.11.100.216 -p_const 28001 -ip_eval localhost -p_eval 28002\n\n";
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
          default_circuit_file.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Circuit representation of shuffle.", // Help description.
          "-c"
  );

  opt.add(
    default_num_iters.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Number of circuits to produce and evaluate.", // Help description.
    "-n"
  );

  opt.add(
    default_execs.c_str(), // Default.
    0, // Required?
    3, // Number of args expected.
    ',', // Delimiter if expecting multiple args.
    "Number of parallel executions for each phase. Preprocessing, Offline and Online.", // Help description.
    "-e"
  );

  opt.add(
    default_ip_address.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "IP Address of Machine running TinyConst for constructor", // Help description.
    "-ip_const"
  );

  opt.add(
    default_port_const.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Port to listen on/connect to for constructor", // Help description.
    "-p_const"
  );

  opt.add(
    default_ram_only.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Use disk", // Help description.
    "-d"
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
  int num_iters, num_execs_components, num_execs_auths, num_execs_online,
          port_const, port_eval_p1, port_eval_p2, ram_only;
  std::vector<int> num_execs;
  std::string circuit_name = "card_shuffle", ip_address_const, ip_address_eval,
          exec_name, circuit_file;
  std::string prefix("eval_");
  opt.get("-c")->getString(circuit_file);
  opt.get("-n")->getInt(num_iters);
  opt.get("-d")->getInt(ram_only);
  circuit_name = prefix + circuit_name;

  opt.get("-e")->getInts(num_execs);
  num_execs_components = num_execs[0];
  num_execs_auths = num_execs[1];
  num_execs_online = num_execs[2];

  opt.get("-ip_const")->getString(ip_address_const);
  opt.get("-p_const")->getInt(port_const);

  ComposedCircuit composed_circuit;

  exec_name = prefix + std::to_string(num_iters) + circuit_file;
  composed_circuit = read_composed_circuit(circuit_file.c_str(), circuit_name);

  //Compute the required number of common_tools that are to be created. We create one main param and one for each sub-thread that will be spawned later on. Need to know this at this point to setup context properly
  int max_num_parallel_execs = max_element(num_execs.begin(), num_execs.end())[0];
  DuploEvaluator duplo_eval(duplo_constant_seeds[1], (uint32_t) max_num_parallel_execs, (bool) ram_only);

  std::cout << "====== EVALUATOR: CONNECTIONG TO CONSTRUCTOR ======" << std::endl;
  duplo_eval.Connect(ip_address_const, (uint16_t) port_const);

  //Values used for network syncing after each phase
  uint8_t rcv;
  uint8_t snd;

  //Run initial Setup (BaseOT) phase
  duplo_eval.Setup();

  std::cout << "====== EVALUATOR: PRE-PROCESSING ======" << std::endl;
  //Run Preprocessing phase
  for (int i = 0; i < composed_circuit.num_functions; ++i) {
    duplo_eval.PreprocessComponentType(composed_circuit.functions[i].circuit_name,
                                       composed_circuit.functions[i],
                                       composed_circuit.num_circuit_copies[i],
                                       (uint32_t) num_execs_components);
  }

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  duplo_eval.PrepareComponents(composed_circuit.num_inp_wires, (uint32_t) num_execs_auths);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  duplo_eval.Build(composed_circuit, (uint32_t) num_execs_auths);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  /*
   * TODO: wait for parties to connect and input.
   * */
  osuCrypto::BitVector seed = GetSeed(SIZE_SEED);
  std::cout << "====== EVALUATOR: CHOSE SEED:  ======" << std::endl;
  PrintHex(seed.data(), SIZE_SEED/8);
  std::cout << std::endl;

  BYTEArrayVector output_keys(composed_circuit.num_out_wires, CSEC_BYTES);
  duplo_eval.Evaluate(composed_circuit, seed, output_keys, (uint32_t) num_execs_online);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  std::cout << "====== EVALUATOR: HAVE BEEN DEALT CARDS: ======" << std::endl;

  std::vector<osuCrypto::BitVector> outputs(composed_circuit.output_circuits.size());
  std::vector<std::vector<uint32_t>> const_output_indices(composed_circuit.output_circuits.size());
  const_output_indices[0] = GetFirstHandIndices(const_first_card_index, HAND_SIZE);

  std::vector<std::vector<uint32_t>> eval_output_indices(composed_circuit.output_circuits.size());
  eval_output_indices[0] = GetFirstHandIndices(eval_first_card_index, HAND_SIZE);

  duplo_eval.DecodeKeys(composed_circuit, const_output_indices, eval_output_indices,
                        output_keys, outputs, true, num_execs_online);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  /*
   * DISPLAYINT FIRST HAND
   * */
  osuCrypto::BitVector hand;
  hand.copy(outputs[0], 0, outputs[0].size());
  hand = Convert6bitTo8BitVector(hand);
  PrintHand(hand.data());

  /*
   * TAKING INPUTS FRO CARDS TO CHANGE
   * */
  std::vector<uint8_t> eval_card_changed = GetCardsToChange();

  std::cout << "====== EVALUATOR: WAITING FOR CONSTRUCTOR TO CHANGE CARDS ======" << std::endl
            << std::endl;

  /*
   * SENDING NUMBER OF CARDS CHANGED BY EVALUATOR.
   * GETTING NUMBER OF CARDS CHANGED BY CONSTRUCTOR.
   * */
  uint8_t num_cards_const_changed[1];
  uint8_t num_cards_eval_changed[1];
  num_cards_eval_changed[0] = eval_card_changed.size();
  duplo_eval.chan.send(num_cards_eval_changed, 1);
  duplo_eval.chan.recv(num_cards_const_changed, 1);

  /*
   * SENDING WHICH CARDS HAVE BEEN CHANGED BY EVALUATOR.
   * GETTING WHICH CARDS HAVE BEEN CHANGED BY CONSTRUCTOR.
   * */
  uint8_t const_card_changed[HAND_SIZE];
  if (num_cards_eval_changed[0] > 0 && num_cards_const_changed[0] > 0) {

    std::cout << "In case 1" << std::endl;

    duplo_eval.chan.send(eval_card_changed.data(), num_cards_eval_changed[0]);
    duplo_eval.chan.recv(&const_card_changed, num_cards_const_changed[0]);
  } else if (num_cards_eval_changed[0] > 0 && num_cards_const_changed[0] == 0) {

    std::cout << "In case 2" << std::endl;

    duplo_eval.chan.send(eval_card_changed.data(), num_cards_eval_changed[0]);
  } else if (num_cards_eval_changed[0] == 0 && num_cards_const_changed[0] > 0) {

    std::cout << "In case 3" << std::endl;

    duplo_eval.chan.recv(&const_card_changed, num_cards_const_changed[0]);
  } else {

    std::cout << "In case 4" << std::endl;

  }

  std::cout << "CONSTRUCTOR CHANGED: \t" << (int) num_cards_const_changed[0] << " CARDS, ";
  for (int j = 0; j < num_cards_const_changed[0]; ++j) {
    std::cout << (int) const_card_changed[j] << " ";
  }
  std::cout << std:: endl;

  std::cout << "EVALUATOR CHANGED: \t" << (int) num_cards_eval_changed[0] << " CARDS, ";
  for (int k = 0; k < num_cards_eval_changed[0]; ++k) {
    std::cout << (int) eval_card_changed[k] << " ";
  }
  std::cout << std:: endl;
  std::cout << std:: endl;


  std::cout << "====== EVALUATOR: FINAL HAND HAVE BEEN DEALT: ======" << std::endl;

  const_output_indices[0] = GetFinalHandIndices(num_cards_const_changed[0],
                                                const_output_indices[0], const_card_changed,
                                                const_first_change_card_index);

  eval_output_indices[0] = GetFinalHandIndices(num_cards_eval_changed[0],
                                               eval_output_indices[0], eval_card_changed.data(),
                                               eval_first_change_card_index);

  duplo_eval.DecodeKeys(composed_circuit, const_output_indices, eval_output_indices,
                        output_keys, outputs, true, num_execs_online);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  /*
   * DISPLAYING EVALUATORS FINAL HAND
   * */
  hand.copy(outputs[0], 0, outputs[0].size());
  hand = Convert6bitTo8BitVector(hand);
  PrintHand(hand.data());


  std::cout << "====== EVALUATOR: CONSTRUCTOR GOT FINAL HAND: ======" << std::endl;

  duplo_eval.DecodeKeys(composed_circuit, eval_output_indices, const_output_indices,
                        output_keys, outputs, true, num_execs_online);

  /*
   * DISPLAYING CONSTRUCTORS FINAL HAND
   * */
  hand.copy(outputs[0], 0, outputs[0].size());
  hand = Convert6bitTo8BitVector(hand);
  PrintHand(hand.data());
}