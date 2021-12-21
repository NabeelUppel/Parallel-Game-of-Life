#!/bin/bash
c++ checkFiles.cpp -o CheckFiles
c++ Serial.cpp -o Serial
mpic++ Parallel.cpp -o Parallel

# shellcheck disable=SC2188
>Results.txt
rm -rf Outputs
mkdir -p "Outputs"

inputs=(Inputs/10x10Test.txt Inputs/20x20Test.txt Inputs/30x30Test.txt Inputs/40x40Test.txt
  Inputs/50x50Test.txt Inputs/100x100Test.txt Inputs/200x200Test.txt Inputs/300x300Test.txt Inputs/400x400Test.txt
  Inputs/500x500Test.txt Inputs/1000x1000Test.txt)

runGameOfLife() {
  echo "${1##*/}" >>Results.txt
  serial=$(./Serial "$i")
  sleep 0.5
  parallel=$(mpirun -oversubscribe -np "$2" --mca opal_warn_on_missing_libcuda 0 ./Parallel "$1")
  ./CheckFiles "$serial" "$parallel" >>Results.txt
  echo -e "\n" >>Results.txt
}

procSize=(2 5 10)
for j in "${procSize[@]}"; do
  echo -e "Number of Processors $j" >>Results.txt
  newName="${j}Results.txt"
  for i in "${inputs[@]}"; do
    runGameOfLife "$i" "$j"
    sleep 1;
  done
  mv Results.txt "${newName}"
done

for f in *.txt; do
  if [[ "$f" == *"Output"* ]]; then
    mv "$f" Outputs/
  fi
  if [[ "$f" == *"Results"* ]]; then
    mv "$f" Results/
  fi
done
