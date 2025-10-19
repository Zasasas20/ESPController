import valveInterface
import sys

match sys.argv[1]:
    case "push":
        if sys.argv[2] == "all": print(valveInterface.openAllValves())
        else: print(valveInterface.openValve(sys.argv[2]))
    case "status":
        if sys.argv[2] == "all": print(valveInterface.getAllValvesStatus())
        else: print(valveInterface.getSingleValveStatus(sys.argv[2]))
    case "enable":
        print(valveInterface.updateValveCount(sys.argv[2]))
    case "set":
        if sys.argv[2] == "all": print(valveInterface.setAllValvesTimer(sys.argv[3]))
        else: print(valveInterface.setValveTimer(sys.argv[2], sys.argv[3]))