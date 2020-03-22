if __name__ == '__main__':
   print("Opening the file") 
   file = open("testout-2725.txt")
   balance_total = 0
   accounts = 0
   ISF_total = 0
   exptected = 995860210
   tok_line = []

   for line in file:
       tok_line = line.split()
       print(tok_line)
    
       if "BAL" in line:
           balance_total += int(tok_line[2])
           accounts += 1
       elif "ISF" in line:
            ISF_total += 1
            print(line)
       elif "5828066" in line:
           print(line)

   print(f"The total accounts is {accounts}, the total balance is {balance_total}," 
            f"and the ISF_total is {ISF_total}")
   print(f"The expected output was {exptected} the difference is {balance_total - exptected}") 
