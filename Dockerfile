# Build uppon DUPLO
FROM cbobach/duplo

# Getting needed files
RUN mkdir -p /home/circuits/cb
WORKDIR "/home/circuits/cb/"
RUN git clone https://github.com/cbobach/speciale_circuit.git .
WORKDIR "/home/src/"
RUN git clone https://github.com/cbobach/speciale_implementation.git tmp
RUN rsync -a tmp/ .
RUN rm -rf tmp

# Compiling Circuit and Poker
WORKDIR "/home/"
RUN ./build/release/Frigate circuits/cb/conditional_swap_shuffle1.wir -dp
RUN ./build/release/Frigate circuits/cb/conditional_swap_shuffle10.wir -dp
RUN ./build/release/Frigate circuits/cb/conditional_swap_shuffle100.wir -dp
RUN ./build/release/Frigate circuits/cb/conditional_swap_shuffle500.wir -dp
RUN ./cmake-release.sh

# Defining entrypoint to trigger DUPLO Poker Constructor, Evaluator or Client
COPY entry-point.sh /home/
RUN chmod 755 /home/entry-point.sh
ENTRYPOINT ["/home/entry-point.sh"]