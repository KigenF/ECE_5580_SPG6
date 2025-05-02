def list_to_string(listto:list) -> str:
    string= ""
    for row in listto:
        string += row.replace("\n", "")
    return string


def keygen_compare(test_file):
    pk_list = []
    with open("PQCkemKAT_16_cryptolReady.rsp", "r") as file:
        save_next = False
        for lines in file:
            if save_next:
                pk_list.append(lines.strip())
                save_next = False
            elif lines.strip() == 'pk':
                save_next = True
        print(len(pk_list))

    compare_list = []
    sublist = []
    with open(test_file, "r") as file:
        save_next = False
        for lines in file:
            sublist.append(lines)
            if lines.strip().find("]") >= 0:
                compare_list.append(list_to_string(sublist))
                sublist = []
        print(len(compare_list))
            
    successes = 0
    for j in zip(pk_list, compare_list):
        # in_line = input(">")
        in_line = str(j[1])
        in_line = in_line.replace(" ", "").upper()
        original = str(j[0]).upper()
        if in_line.strip() == original.strip():
            successes+=1
    print(f"{successes}/100")



keygen_compare("message-full.txt")