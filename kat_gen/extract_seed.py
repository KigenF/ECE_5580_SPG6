def extract_seeds(input_file, output_file):
    seeds = []
    with open(input_file, "r") as fin:
        for line in fin:
            line = line.strip()
            # "seed =" で始まる行を検出
            if line.startswith("seed ="):
                # "=" 以降の部分を抽出して、空白を除去
                _, seed_value = line.split("=", 1)
                seeds.append(seed_value.strip())
    
    # 抽出した seed を1行ずつ出力
    with open(output_file, "w") as fout:
        for seed in seeds:
            fout.write(seed + "\n")


if __name__ == "__main__":
    extract_seeds("PQCkemKAT_16.req", "PQCkemKAT_16-seed.txt")