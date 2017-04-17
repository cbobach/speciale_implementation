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
    default_ip_address.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "IP Address of Machine running TinyConst for evaluator", // Help description.
    "-ip_eval"
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
    default_port_eval_p1.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Port to listen on/connect to for evaluator", // Help description.
    "-p_eval_p1"
  );

  opt.add(
          default_port_eval_p2.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port to listen on/connect to for evaluator", // Help description.
          "-p_eval_p2"
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
  std::string circuit_name = "card_shuffle", ip_address_const, ip_address_eval, exec_name, circuit_file = "circuits/cb/conditional_swap_shuffle.wir.GC_duplo";
  std::string prefix("eval_");

  opt.get("-n")->getInt(num_iters);
  opt.get("-d")->getInt(ram_only);
  circuit_name = prefix + circuit_name;

  opt.get("-e")->getInts(num_execs);
  num_execs_components = num_execs[0];
  num_execs_auths = num_execs[1];
  num_execs_online = num_execs[2];

  opt.get("-ip_const")->getString(ip_address_const);
  opt.get("-ip_eval")->getString(ip_address_eval);
  opt.get("-p_const")->getInt(port_const);
  opt.get("-p_eval_p1")->getInt(port_eval_p1);
  opt.get("-p_eval_p2")->getInt(port_eval_p2);

  ComposedCircuit composed_circuit;

  exec_name = prefix + std::to_string(num_iters) + circuit_file;
  composed_circuit = read_composed_circuit(circuit_file.c_str(), circuit_name);

  //Compute the required number of common_tools that are to be created. We create one main param and one for each sub-thread that will be spawned later on. Need to know this at this point to setup context properly
  int max_num_parallel_execs = max_element(num_execs.begin(), num_execs.end())[0];
  DuploEvaluator duplo_eval(duplo_constant_seeds[1], (uint32_t) max_num_parallel_execs, (bool) ram_only);
  duplo_eval.Connect(ip_address_const, (uint16_t) port_const);

  std::cout << "====== PRE-PROCESSING ======" << std::endl;

  //Values used for network syncing after each phase
  uint8_t rcv;
  uint8_t snd;

  //Run initial Setup (BaseOT) phase
  duplo_eval.Setup();

  //Run Preprocessing phase
  for (int i = 0; i < composed_circuit.num_functions; ++i) {
    duplo_eval.PreprocessComponentType(composed_circuit.functions[i].circuit_name, composed_circuit.functions[i], composed_circuit.num_circuit_copies[i],
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

  std::cout << "====== WAITING FOR PARTY 1 TO CONNECT ON PORT: " << port_eval_p1 << "  ======" << std::endl;

  osuCrypto::IOService ios_eval;

  osuCrypto::Endpoint ep_p1;
  ep_p1.start(ios_eval, ip_address_eval, (osuCrypto::u32) port_eval_p1, osuCrypto::EpMode::Server, "ep_eval_p1");

  osuCrypto::Channel chan_p1 = ep_p1.addChannel("eval", "eval");
  uint8_t input_p1[SIZE_SEED];
  chan_p1.recv(input_p1, SIZE_SEED);

  std::cout << "====== WAITING FOR PARTY 2 TO CONNECT ON PORT: " << port_eval_p2 << "  ======" << std::endl;

  osuCrypto::Endpoint ep_p2;
  ep_p2.start(ios_eval, ip_address_eval, (osuCrypto::u32) port_eval_p2, osuCrypto::EpMode::Server, "ep_eval_p2");

  osuCrypto::Channel chan_p2 = ep_p2.addChannel("eval", "eval");
  uint8_t input_p2[SIZE_SEED];
  chan_p2.recv(input_p2, SIZE_SEED);

  /*
   * Constructing input vector for circuit evaluation
   * */
  osuCrypto::BitVector i1(input_p1, sizeof(input_p1));
  osuCrypto::BitVector i2(input_p2, sizeof(input_p2));

  osuCrypto::BitVector inputs = i1;
  inputs.reserve(i1.size() + i2.size());
  inputs.append(i2);

  BYTEArrayVector output_keys(composed_circuit.num_out_wires, CSEC_BYTES);

  duplo_eval.Evaluate(composed_circuit, inputs, output_keys, (uint32_t) num_execs_online);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);


  std::vector<osuCrypto::BitVector> outputs(composed_circuit.output_circuits.size());
  std::vector<std::vector<uint32_t>> const_output_indices = composed_circuit.GetOutputIndices(false);
  std::vector<std::vector<uint32_t>> eval_output_indices = composed_circuit.GetOutputIndices(true);

  duplo_eval.DecodeKeys(composed_circuit, const_output_indices, eval_output_indices, output_keys, outputs,
                        (bool) num_execs_online);

  //Sync with Constructor
  duplo_eval.chan.send(&snd, 1);
  duplo_eval.chan.recv(&rcv, 1);

  std::cout << "====== SENDING HAND ======" << std::endl;

  /*
   * TODO: Send correct indexes!!!
   *       5 secret shares to P1
   *       5 next secret shares to P2
   * */
  chan_p1.send(outputs.data()->data(), HAND_SIZE);
  chan_p2.send(outputs.data()->data(), HAND_SIZE);
}