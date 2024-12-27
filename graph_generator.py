import pandas as pd
import subprocess
import matplotlib.pyplot as plt
import sys
import copy



# predictor = "smith"
# if len(sys.argv) > 1:
#     predictor = sys.argv[1]

predictors_list = ["gshare"]            # ["smith", "bimodal", "gshare"]
benchmark_files = ["gcc_trace.txt", "perl_trace.txt", "jpeg_trace.txt"]

smith_B = [1, 2, 3, 4, 5, 6]
bimodal_m = [7, 8, 9, 10, 11, 12]       # same for gshare


for predictor in predictors_list:
    for benchmarks in benchmark_files:
        print("working on benchmark file : ", benchmarks, "and predictor is : ", predictor)
        if predictor == "smith":
            Graph_title = benchmarks.split('.')[0] + ", smith"
            Y_axis = []
            X_axis = []
            for B in smith_B:            
                process = subprocess.Popen(
                    ["sim", 
                    "smith", 
                    str(B), 
                    benchmarks], 
                    stdout=subprocess.PIPE,  # Capture standard output
                    stderr=subprocess.PIPE   # Capture standard error
                )

                # Read the output
                stdout, stderr = process.communicate()

                # Decode the byte output to string
                stdout = stdout.decode('utf-8')
                stderr = stderr.decode('utf-8')
                result = stdout
                # print(result)

                # miss_pred_rate = float(str(result).splitlines()[0])
                miss_pred_rate = float(str(str(result).splitlines()[2]).split(':')[1].split('%')[0].strip())      # miss pred rate is at 3rd(0,1,2nd) line and after ':'
                X_axis.append(B)
                Y_axis.append(miss_pred_rate)

            print("Graph: ", Graph_title)
            print("smith counter B values:", X_axis)
            print("smith miss prediction rate", Y_axis)
            # Now plot the graph
            plt.plot(X_axis, Y_axis, label="smith_counter", marker='o', linestyle='-', color='b')

            plt.xlabel("smith_b")
            plt.ylabel("branch miss prediction rate")
            plt.title(Graph_title)
            # Show legend
            plt.legend()
            # Show the plot
            plt.show()


        elif predictor == "bimodal":
            Graph_title = benchmarks.split('.')[0] + ", bimodal"
            Y_axis = []
            X_axis = []
            for M in bimodal_m:            
                process = subprocess.Popen(
                    ["sim", 
                    "bimodal", 
                    str(M), 
                    benchmarks], 
                    stdout=subprocess.PIPE,  # Capture standard output
                    stderr=subprocess.PIPE   # Capture standard error
                )

                # Read the output
                stdout, stderr = process.communicate()

                # Decode the byte output to string
                stdout = stdout.decode('utf-8')
                stderr = stderr.decode('utf-8')
                result = stdout
                # print(result)

                # miss_pred_rate = float(str(result).splitlines()[0])
                miss_pred_rate = float(str(str(result).splitlines()[2]).split(':')[1].split('%')[0].strip())      # miss pred rate is at 3rd(0,1,2nd) line and after ':'
                X_axis.append(M)
                Y_axis.append(miss_pred_rate)


            print("Graph: ", Graph_title)
            print("bimodal predictor M values:", X_axis)
            print("bimodal miss prediction rate", Y_axis)
            # Now plot the graph
            plt.plot(X_axis, Y_axis, label="bimodal_predictor", marker='o', linestyle='-', color='b')

            plt.xlabel("bimodal_m")
            plt.ylabel("branch miss prediction rate")
            plt.title(Graph_title)
            # Show legend
            plt.legend()
            # Show the plot
            plt.show()


        elif predictor == "gshare":
            Graph_title = benchmarks.split('.')[0] + ", gshare"
            Value_dict = {}
            for M in bimodal_m:  
                N = 2    
                while N <= M:       
                    process = subprocess.Popen(
                        ["sim", 
                        "gshare", 
                        str(M), 
                        str(N),
                        benchmarks], 
                        stdout=subprocess.PIPE,  # Capture standard output
                        stderr=subprocess.PIPE   # Capture standard error
                    )

                    # Read the output
                    stdout, stderr = process.communicate()

                    # Decode the byte output to string
                    stdout = stdout.decode('utf-8')
                    stderr = stderr.decode('utf-8')
                    result = stdout
                    # print(result)

                    # miss_pred_rate = float(str(result).splitlines()[0])
                    miss_pred_rate = float(str(str(result).splitlines()[2]).split(':')[1].split('%')[0].strip())      # miss pred rate is at 3rd(0,1,2nd) line and after ':'
                    if N in Value_dict:
                        Value_dict[N][M] = miss_pred_rate
                    else:
                        dict_ = {}
                        dict_[M] = miss_pred_rate
                        Value_dict[N] = dict_

                    N += 2

            for n in Value_dict.keys():
                m_dict = Value_dict[n]
                X_axis = [elem for elem in m_dict.keys()]
                Y_axis = [elm for elm in m_dict.values()]
                
                print("Graph: ", Graph_title)
                print("gshare : n = ", n, " predictor's M values:", X_axis)
                print("gshare : n = ", n, " miss prediction rate", Y_axis)
                # Now plot the graph
                plt.plot(X_axis, Y_axis, marker='o', label=f"gshare_n={n}")     # , marker='o', linestyle='-', color='b')

            plt.xlabel("gshare_m")
            plt.ylabel("branch miss prediction rate")
            plt.title(Graph_title)
            # Show legend
            plt.legend()
            # Show the plot
            plt.show()












