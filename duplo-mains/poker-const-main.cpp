#include "duplo-mains/poker-mains.h"
#include "duplo/duplo-constructor.h"

int main(int argc, const char* argv[]) {
  ezOptionParser opt;

  opt.overview = "PokerConstructor Passing Parameters Guide.";
  opt.syntax = "PokerConstructor first second third forth fifth sixth";
  opt.example = "PokerConstructor -f path/to/file.wir.GC_duplo -e 8,2,1 -ip_const 10.11.100.216 -p_const 28001 -n 1 -i 1\n\n";
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
          "Circuitfile representation of shuffle.", // Help description.
          "-f"
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
          default_num_iters.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Number of parallel circuits", // Help description.
          "-n"
  );

  opt.add(
          default_interactive.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Interactive", // Help description.
          "-i"
  );

  opt.add(
          default_ip_address.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "IP Address of Constructor", // Help description.
          "-ip_const"
  );

  opt.add(
          default_port_const.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port to listen on for Constructor", // Help description.
          "-p_const"
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
  std::vector<int> num_execs;
  int num_iters, num_execs_components, num_execs_auths, num_execs_online,
          port_const, ram_only = 0, interactive;
  std::string circuit_name = "card_shuffle", ip_address_const, exec_name,
          circuit_file;

  std::string prefix("const_");
  opt.get("-f")->getString(circuit_file);
  circuit_name = prefix + circuit_name;

  opt.get("-e")->getInts(num_execs);
  num_execs_components = num_execs[0];
  num_execs_auths = num_execs[1];
  num_execs_online = num_execs[2];

  opt.get("-ip_const")->getString(ip_address_const);
  opt.get("-p_const")->getInt(port_const);

  opt.get("-n")->getInt(num_iters);
  opt.get("-i")->getInt(interactive);

  //Set the circuit variables according to circuit_name
  ComposedCircuit composed_circuit;
  exec_name = prefix + std::to_string(num_iters) + circuit_file;
  composed_circuit = read_composed_circuit(circuit_file.c_str(), circuit_name);


  //Compute the required number of common_tools that are to be created. We create one main param and one for each sub-thread that will be spawned later on. Need to know this at this point to setup context properly
  int max_num_parallel_execs = max_element(num_execs.begin(), num_execs.end())[0];
  DuploConstructor duplo_const(duplo_constant_seeds[0], (uint32_t) max_num_parallel_execs, (bool) ram_only);


  std::cout << "====== CONSTRUCTOR: WAITING FOR EVALUATOR ON, "
            << "PORT: " << port_const << " ======" << std::endl;
  duplo_const.Connect(ip_address_const, (uint16_t) port_const);

  //Values used for network syncing after each phase
  uint8_t rcv;
  uint8_t snd;

  //Run initial Setup (BaseOT) phase
  auto setup_begin = GET_TIME();
  duplo_const.Setup();
  auto setup_end = GET_TIME();
  uint64_t setup_data_sent = duplo_const.GetTotalDataSent();

  std::cout << "====== CONSTRUCTOR: PRE-PROCESSING ======" << std::endl;
  //Run Preprocessing phase
  auto preprocess_begin = GET_TIME();
  for (int i = 0; i < composed_circuit.num_functions; ++i) {
    duplo_const.PreprocessComponentType(composed_circuit.functions[i].circuit_name,
                                        composed_circuit.functions[i],
                                        composed_circuit.num_circuit_copies[i],
                                        (uint32_t) num_execs_components);
  }
  auto preprocess_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan.recv(&rcv, 1);
  duplo_const.chan.send(&snd, 1);
  uint64_t preprocess_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent;

  auto prepare_eval_begin = GET_TIME();
  duplo_const.PrepareComponents(composed_circuit.num_inp_wires, (uint32_t) num_execs_auths);
  auto prepare_eval_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan.recv(&rcv, 1);
  duplo_const.chan.send(&snd, 1);
  uint64_t prepare_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent;

  auto build_begin = GET_TIME();
  duplo_const.Build(composed_circuit, num_execs_auths);
  auto build_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan.recv(&rcv, 1);
  duplo_const.chan.send(&snd, 1);
  uint64_t build_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent;

  /*
   * CHOOSING SEED
   * */
  osuCrypto::BitVector seed = GetSeed(composed_circuit.num_const_inp_wires);
  std::cout << "====== CONSTRUCTOR: CHOSE SEED: ======" << std::endl;
  PrintHex(seed.data(), composed_circuit.num_const_inp_wires / 8);
  std::cout << std::endl;

  auto eval_circuits_begin = GET_TIME();
  duplo_const.Evaluate(composed_circuit, seed, (uint32_t) num_execs_online);
  auto eval_circuits_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan.recv(&rcv, 1);
  duplo_const.chan.send(&snd, 1);
  uint64_t eval_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent;


  auto decode_keys_first_hand_begin = GET_TIME(),
          decode_keys_first_hand_end = GET_TIME(),
          decode_keys_final_hand_begin = GET_TIME(),
          decode_keys_final_hand_end = GET_TIME(),
          decode_keys_oponent_hand_begin = GET_TIME(),
          decode_keys_oponent_hand_end = GET_TIME();

  uint64_t
          decode_data_first_hand_sent,
          decode_data_final_hand_sent,
          decode_data_oponent_hand_sent;

  uint64_t
          first_hand_nano = 0,
          final_hand_nano = 0,
          oponent_hand_nano = 0,
          first_hand_sent = 0,
          final_hand_sent = 0,
          oponent_hand_sent = 0;

  for (int l = 0; l < num_iters; ++l) {
    std::cout << "====== CONSTRUCTOR: HAVE BEEN DEALT CARDS: ======" << std::endl;

    /*
     * GENERATING WHICH WIRES TO OPEN TO WHICH PARTY
     * */
    std::vector<osuCrypto::BitVector> outputs(composed_circuit.output_circuits.size());
    std::vector<std::vector<uint32_t>> const_output_indices(composed_circuit.output_circuits.size());
    const_output_indices[l] = GetFirstHandIndices(const_first_card_index, HAND_SIZE);

    std::vector<std::vector<uint32_t>> eval_output_indices(composed_circuit.output_circuits.size());
    eval_output_indices[l] = GetFirstHandIndices(eval_first_card_index, HAND_SIZE);


    decode_keys_first_hand_begin = GET_TIME();
    duplo_const.DecodeKeys(composed_circuit, const_output_indices, eval_output_indices, outputs,
                           true, num_execs_online);
    decode_keys_first_hand_end = GET_TIME();


    //Sync with Evaluator
    duplo_const.chan.recv(&rcv, 1);
    duplo_const.chan.send(&snd, 1);

    decode_data_first_hand_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent - eval_data_sent;

    first_hand_nano += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_keys_first_hand_end - decode_keys_first_hand_begin).count();
    first_hand_sent += decode_data_first_hand_sent;

    /*
     * DISPLAYING FIRST HAND
     * */
    osuCrypto::BitVector hand;
    hand.copy(outputs[l], 0, outputs[l].size());
    hand = Convert6bitTo8BitVector(hand);
    PrintHand(hand.data());

    /*
     * RECEIVING INPUTS FOR CARDS TO CHANGE
     * */
    std::vector<uint8_t> const_card_changed;
    if(interactive)
      const_card_changed = GetCardsToChange();

    std::cout << "====== CONSTRUCTOR: WAITING FOR EVALUATOR TO CHANGE CARDS ======" << std::endl
              << std::endl;

    /*
     * RECEIVING NUMBERS OF CARDS CHANGED BY EVALUATOR.
     * SENDING NUMBER OF CARDS CHANGED BY CONSTRUCTOR
     * */
    uint8_t num_cards_const_changed[1];
    uint8_t num_cards_eval_changed[1];
    num_cards_const_changed[0] = const_card_changed.size();

    duplo_const.chan.recv(num_cards_eval_changed, 1);
    duplo_const.chan.send(num_cards_const_changed, 1);

    /*
     * RECEIVING WHICH CARDS HAVE BEEN CHANGED BY EVALUATOR.
     * SENDING WHICH CARDS HAVE BEEN CHANGED BY CONSTRUCTOR
     * */
    uint8_t eval_card_changed[HAND_SIZE];
    if (num_cards_eval_changed[0] > 0 && num_cards_const_changed[0] > 0) {
      duplo_const.chan.recv(&eval_card_changed, num_cards_eval_changed[0]);
      duplo_const.chan.send(const_card_changed.data(), num_cards_const_changed[0]);
    } else if (num_cards_eval_changed[0] > 0 && num_cards_const_changed[0] == 0) {
      duplo_const.chan.recv(&eval_card_changed, num_cards_eval_changed[0]);
    } else if (num_cards_eval_changed[0] == 0 && num_cards_const_changed[0] > 0) {
      duplo_const.chan.send(const_card_changed.data(), num_cards_const_changed[0]);
    }

    std::cout << "CONSTRUCTOR CHANGED: \t" << (int) num_cards_const_changed[0] << " CARDS, ";
    for (int j = 0; j < num_cards_const_changed[0]; ++j) {
      std::cout << (int) const_card_changed[j] << " ";
    }
    std::cout << std::endl;

    std::cout << "EVALUATOR CHANGED: \t" << (int) num_cards_eval_changed[0] << " CARDS, ";
    for (int k = 0; k < num_cards_eval_changed[0]; ++k) {
      std::cout << (int) eval_card_changed[k] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "====== CONSTRUCTOR: FINAL HAND HAVE BEEN DEALT: ======" << std::endl;

    /*
     * GENERATING WHICH WIRES TO OPEN TO WHICH PARTY
     * */
    const_output_indices[l] = GetFinalHandIndices(num_cards_const_changed[0],
                                                  const_output_indices[0], const_card_changed.data(),
                                                  const_first_change_card_index);

    eval_output_indices[l] = GetFinalHandIndices(num_cards_eval_changed[0],
                                                 eval_output_indices[0], eval_card_changed,
                                                 eval_first_change_card_index);

    decode_keys_final_hand_begin = GET_TIME();
    duplo_const.DecodeKeys(composed_circuit, const_output_indices, eval_output_indices, outputs,
                           true, num_execs_online);
    decode_keys_final_hand_end = GET_TIME();

    //Sync with Evaluator
    duplo_const.chan.recv(&rcv, 1);
    duplo_const.chan.send(&snd, 1);

    decode_data_final_hand_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent - eval_data_sent - decode_data_first_hand_sent;

    final_hand_nano += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_keys_final_hand_end - decode_keys_final_hand_begin).count();
    final_hand_sent += decode_data_final_hand_sent;

    /*
     * DISPLAYING CONSTRUCTORS FINAL HAND
     * */
    hand.copy(outputs[l], 0, outputs[l].size());
    hand = Convert6bitTo8BitVector(hand);
    PrintHand(hand.data());


    std::cout << "====== CONSTRUCTOR: EVALUATOR GOT FINAL HAND: ======" << std::endl;

    decode_keys_oponent_hand_begin = GET_TIME();
    duplo_const.DecodeKeys(composed_circuit, eval_output_indices, const_output_indices,
                           outputs, true, num_execs_online);
    decode_keys_oponent_hand_end = GET_TIME();

    //Sync with Evaluator
    duplo_const.chan.recv(&rcv, 1);
    duplo_const.chan.send(&snd, 1);

    decode_data_oponent_hand_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent - eval_data_sent - decode_data_first_hand_sent - decode_data_final_hand_sent;

    oponent_hand_nano += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_keys_oponent_hand_end - decode_keys_oponent_hand_begin).count();
    oponent_hand_sent += decode_data_oponent_hand_sent;

    /*
     * DISPLAYING EVALUATORS FINAL HAND
     * */
    hand.copy(outputs[l], 0, outputs[l].size());
    hand = Convert6bitTo8BitVector(hand);
    PrintHand(hand.data());
  }



  // Average out the timings of each phase and print results
  uint64_t setup_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(setup_end - setup_begin).count();
  uint64_t preprocess_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(preprocess_end - preprocess_begin).count();

  uint64_t build_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(build_end - build_begin).count();
  uint64_t prepare_eval_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(prepare_eval_end - prepare_eval_begin).count();

  uint64_t eval_circuits_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(eval_circuits_end - eval_circuits_begin).count();

  std::cout << "=============================" << std::endl;
  std::cout
          << "\t"
          << "#Decks:\t"
          << "Setup:\t\t"
          << "Circuit Preprocess:\t"
          << "Auth Preprocess:\t"
          << "Build:\t"
          << "Eval:\t"
          << "Decode first:\t"
          << "Decode final:\t"
          << "Decode oponent:\t"
          << std::endl;

  std::cout << std::setprecision(2);
  std::cout << std::fixed;

  std::cout
          << "ms,\t"
          << num_iters << ",\t"
          << (double) setup_time_nano / num_iters / 1000000 << ",\t"
          << (double) preprocess_time_nano / num_iters / 1000000 << ",\t\t"
          << (double) prepare_eval_time_nano / num_iters / 1000000 << ",\t\t\t"
          << (double) build_time_nano / num_iters / 1000000 << ",\t"
          << (double) eval_circuits_nano / num_iters / 1000000 << ",\t"
          << (double) first_hand_nano / num_iters / 1000000 << ",\t\t"
          << (double) final_hand_nano / num_iters / 1000000 << ",\t\t"
          << (double) oponent_hand_nano / num_iters / 1000000
          << std::endl;

  std::ofstream ms_file ("const_ms.log", std::ios_base::app);
  if (ms_file.is_open()){
    ms_file
            << num_iters << ", "
            << (double) setup_time_nano / num_iters / 1000000 << ", "
            << (double) preprocess_time_nano / num_iters / 1000000 << ", "
            << (double) prepare_eval_time_nano / num_iters / 1000000 << ", "
            << (double) build_time_nano / num_iters / 1000000 << ", "
            << (double) eval_circuits_nano / num_iters / 1000000 << ", "
            << (double) first_hand_nano / num_iters / 1000000 << ", "
            << (double) final_hand_nano / num_iters / 1000000 << ", "
            << (double) oponent_hand_nano / num_iters / 1000000
            << std::endl;
  } else {
    std::cout << "Could not write ms to log file!" << std::endl;
  }
  ms_file.close();

  std::cout
          << "kb,\t"
          << num_iters << ",\t"
          << (double) setup_data_sent / num_iters / 1000 << ",\t\t"
          << (double) preprocess_data_sent / num_iters / 1000 << ",\t\t"
          << (double) prepare_data_sent / num_iters / 1000 << ",\t\t"
          << (double) build_data_sent / num_iters / 1000 << ",\t"
          << (double) eval_data_sent / num_iters / 1000 << ",\t"
          << (double) first_hand_sent / num_iters / 1000 << ",\t\t"
          << (double) final_hand_sent / num_iters / 1000 << ",\t\t"
          << (double) oponent_hand_sent / num_iters / 1000
          << std::endl;

  std::ofstream kb_file ("const_kb.log", std::ios_base::app);
  if (kb_file.is_open()){
    kb_file
            << num_iters << ", "
            << (double) setup_data_sent / num_iters / 1000 << ", "
            << (double) preprocess_data_sent / num_iters / 1000 << ", "
            << (double) prepare_data_sent / num_iters / 1000 << ", "
            << (double) build_data_sent / num_iters / 1000 << ", "
            << (double) eval_data_sent / num_iters / 1000 << ", "
            << (double) first_hand_sent / num_iters / 1000 << ", "
            << (double) final_hand_sent / num_iters / 1000 << ", "
            << (double) oponent_hand_sent / num_iters / 1000
            << std::endl;
  }else {
    std::cout << "Could not write kb to log file!" << std::endl;
  }
  kb_file.close();

  std::cout << "=============================" << std::endl;
}
