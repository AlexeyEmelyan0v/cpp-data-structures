test_answer = open("suggested_output.txt", "r")
program_answer = open("output.txt", "r")
flag = 0
l = 0
for i in test_answer:
    l += 1
    che = program_answer.readline()
    if i != che:
        print(i[:-1])
        print(che[:-1])


test_answer.close()
program_answer.close()