import sys

if __name__ == '__main__':
   print("Opening the file") 
   fname = sys.argv[1]
   file = open(fname)
   balance_total = 0
   accounts = 0
   ISF_total = 0
   exptected = 995860210
   tok_line = []

   for line in file:
       tok_line = line.split()
    
       if "BAL" in line:
           balance_total += int(tok_line[2])
           accounts += 1
       elif "ISF" in line:
            ISF_total += 1
            print(line)

   print(f"The total accounts is {accounts}, the total balance is {balance_total}," 
            f"and the ISF_total is {ISF_total}")
   print(f"The expected output was {exptected} the difference is {balance_total - exptected}") 
