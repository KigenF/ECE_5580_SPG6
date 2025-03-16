def extract_seeds(input_file, output_file):
    seeds = []
    with open(input_file, "r") as fin:
        for line in fin:
            line = line.strip()
            # detect line "seed =" 
            if line.startswith("seed ="):
                # extract after "=" 
                _, seed_value = line.split("=", 1)
                seeds.append(seed_value.strip())
    
    # output seed
    with open(output_file, "w") as fout:
        for seed in seeds:
            fout.write(seed + "\n")


if __name__ == "__main__":
    extract_seeds("Original_PQCkemKAT_16.req", "PQCkemKAT_16-seed.txt")
