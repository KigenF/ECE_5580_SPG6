from textwrap import wrap

def prep_file(file_name):
    with open(file_name, "r", encoding="utf-8") as rsp_file:
        outFile = file_name.split(".")[0] + "_cryptolReady." + file_name.split(".")[1]
        alt_file = open(outFile, "w", encoding="utf-8")
        for line in rsp_file:
            if line.strip() != "":
                label = line.split(" = ")[0]
                if label == "count":
                    alt_file.write(line)
                else:
                    data_line = "["
                    if len(line.split(" = ")) > 1:
                        for i in wrap(line.split(" = ")[1],2):
                            data_line = data_line + "0x" + i + ','
                        data_line = data_line[:-1] + "]"
                        alt_file.write((label + "\n" + data_line + "\n"))
                    else:
                        alt_file.write((label + "\n"))
        alt_file.close()


if __name__ == "__main__":
    prep_file("PQCkemKAT_16.req")
    prep_file("PQCkemKAT_16.rsp")