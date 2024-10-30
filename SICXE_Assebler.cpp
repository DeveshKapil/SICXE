#include<iostream>
#include<unordered_map>
#include<string>
#include<iomanip>
#include<fstream>
#include<sstream>
#include<queue>

using namespace std;


string intToHex(int value , int n) {
    stringstream ss;
    ss << hex << setw(n) << setfill('0') << value;
    return ss.str();
}


void seperate(string &line , string &label , string &opcode , string &operand)
{
    stringstream ss(line);
    if (line[0] != ' ' && line[0] != '\t') {
        ss >> label;
    } else {
        label = "";
    }
    ss >> opcode >> operand;
}

void print(ofstream &ofile , int &locctr , int &start , string &objCode , int &count , queue<string> &qu)
{
    ofile << "T" << intToHex(start , 6) << " " << intToHex(count , 2) << " " << objCode << endl ;
    while(!qu.empty())
    {
        ofile << "M " << qu.front() << " 05" << endl;
        qu.pop();
    }
    objCode = "";
    start = locctr;
    count = 0;
}

int firstPass(ifstream &ifile , int &pstart , unordered_map<string , string> &optab , unordered_map<string , int> &symtab)
{
    string line , label , opcode , operand;
    int locctr = 0 , i=1;

    while(getline(ifile , line))
    {
        seperate(line , label , opcode , operand);
        if (opcode == "START")
        {
            pstart = stoi(operand , nullptr , 16);
            locctr = pstart;
        }
        else
        {
            if(!label.empty())
                if(symtab.find(label) == symtab.end())
                    symtab[label] = locctr;
                else
                {
                    cerr << "Dulpicate symbol "<< label << " found on line " << i << endl;
                    return -1;
                }
            if(opcode == "WORD")
                locctr += 3;
            else if(opcode == "RESW")
                locctr += 3*stoi(operand);
            else if(opcode == "RESB")
                locctr += stoi(operand);
            else if(opcode == "BYTE")
                locctr += operand.length()-3;
            else if(opcode[0] == '+')
                locctr += 4;
            else
                if(optab.find(opcode) != optab.end())
                    locctr += 3;
                else
                {
                    cerr << "Opcode defined on line " << i <<" " << opcode << " was not found in the optab." << endl;
                    return -1;
                }
        }
        i++;
    }
    return locctr - pstart;
}

int secondPass(ifstream &ifile ,ofstream &ofile , int &pstart , int &plen, unordered_map<string , string> &optab , unordered_map<string , int> &symtab)
{
    string line, label, opcode, operand , objCode;
    int locctr = pstart , i=1 , counter = 0;
    int start = locctr;
    int progcounter , base = -1;;
    queue<string> qu;

    getline(ifile , line);
    seperate(line , label , opcode , operand);

    if(opcode == "START")
        ofile << "H" << label << setw(6) <<  ' ' << intToHex(pstart , 6) << ' ' << intToHex(plen , 6) << endl;

    while(getline(ifile , line))
    {
        seperate(line, label, opcode, operand);

        if(opcode == "END") break;
        else if (opcode == "RESW" || opcode == "RESB") 
        {
            if(objCode.length()>0)
            {
                print(ofile , locctr , start , objCode , counter , qu);
            }
            if (opcode == "RESW")
            {
                locctr += 3*stoi(operand);
            }
            locctr += stoi(operand);
        }
        else if(opcode == "BASE")
        {
            base = symtab[operand];
            locctr += 3;
        }
        else if (opcode == "NOBASE")
        {
            base = -1;
            locctr += 3;
        }
        else
        {
            if(objCode.length() == 0)
            {
                start = locctr;
            }
            if(opcode == "BYTE")
            {
                string temp = operand.substr(2,operand.length()-3);
                int len = temp.length();
                if(counter+len/2 > 30) 
                {
                    print(ofile , locctr , start , objCode , counter , qu);
                }
                objCode += temp;
                locctr += len/2;
                counter += len/2;
                
            }
            else if (opcode == "WORD")
            {
                string temp = intToHex(stoi(operand) , 6);
                if(counter+3 > 30) 
                {
                    print(ofile , locctr , start , objCode , counter , qu);
                }
                objCode += temp;
                locctr += 3;
                counter += 3;
            }

            else
            {
                if(opcode[0] == '+')
                {
                    int temp = stoi(optab[opcode.substr(1)]);
                    if(symtab.find(operand)!=symtab.end())
                    {
                        temp += intToHex(symtab[operand] , 4);
                        
                    }
                    else
                    {
                        cerr << "Operand defined on line " << i << " " << operand << " was not found in the symtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    if(counter+4 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter , qu);
                    }
                    objCode += temp;
                    locctr += 4;
                    counter += 4;
                }
                else{
                    string temp = optab[opcode];
                    int disp;
                    if(operand.length() == 0)
                    {
                        temp += intToHex(0 , 4);
                    }
                    else if(symtab.find(operand)!=symtab.end())
                    {
                        disp = symtab[operand] - locctr - 3;
                        if((disp < 2048 || disp > 2047) && base == -1) 
                        {
                            cerr << "Addresss out of range for format 3 addressing in line " << i << endl;
                            return -1;
                        }
                        else if( )
                        {
                            if(disp > 2047)
                                temp += 
                        }
                        temp += intToHex(symtab[operand] , 4);
                        temp -= 
                    }
                    else
                    {
                        cerr << "Operand defined on line " << i << " " << operand << " was not found in the symtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    if(counter+3 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter , qu);
                    }
                    objCode += temp;
                    locctr += 3;
                    counter += 3;
                }
            }
        }
        
        i++;
    }
    if(objCode.length()>0)
    {
        print(ofile , locctr , start , objCode , counter , qu);
    }
    ofile << "E" << intToHex(pstart , 6) << endl;
    return 0;
}


int main()
{
    unordered_map<string , string> optab = {
        {"LDA", "00"}, {"LDX", "04"}, {"LDL", "08"}, {"STA", "0C"}, {"STX", "10"}, {"STL", "14"},
        {"ADD", "18"}, {"SUB", "1C"}, {"MUL", "20"}, {"DIV", "24"}, {"COMP", "28"}, {"TIX", "2C"},
        {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"}, {"J", "3C"}, {"AND", "40"}, {"OR", "44"},
        {"JSUB", "48"}, {"RSUB", "4C"}, {"LDCH", "50"}, {"STCH", "54"}, {"ADDF", "58"}, {"SUBF", "5C"},
        {"MULF", "60"}, {"DIVF", "64"}, {"LDB", "68"}, {"LDS", "6C"}, {"LDF", "70"}, {"LDT", "74"},
        {"STB", "78"}, {"STS", "7C"}, {"STF", "80"}, {"STT", "84"}, {"COMPF", "88"}, {"ADDR", "90"},
        {"SUBR", "94"}, {"MULR", "98"}, {"DIVR", "9C"}, {"TIXR", "B8"}, {"CLEAR", "B4"}, {"SHIFTL", "A4"},
        {"SHIFTR", "A8"}, {"SVC", "B0"}, {"FLOAT", "C0"}, {"FIX", "C4"}, {"NORM", "C8"}, {"LPS", "D0"},
        {"STSW", "E8"}, {"RD", "D8"}, {"WD", "DC"}, {"TD", "E0"}, {"SSK", "EC"}, {"STI", "D4"},
        {"ADDF", "58"}, {"SUBF", "5C"}, {"MULF", "60"}, {"DIVF", "64"}, {"COMPF", "88"}, {"LDB", "68"},
        {"LDS", "6C"}, {"LDF", "70"}, {"LDT", "74"}, {"STB", "78"}, {"STS", "7C"}, {"STF", "80"},
        {"STT", "84"}, {"COMPF", "88"}, {"ADDR", "90"}, {"SUBR", "94"}, {"MULR", "98"}, {"DIVR", "9C"},
        {"CLEAR", "B4"}, {"TIXR", "B8"}, {"SHIFTL", "A4"}, {"SHIFTR", "A8"}, {"SVC", "B0"}, {"FLOAT", "C0"},
        {"FIX", "C4"}, {"NORM", "C8"}, {"LPS", "D0"}, {"STI", "D4"}, {"RD", "D8"}, {"WD", "DC"},
        {"TD", "E0"}, {"SSK", "EC"}
    };
    unordered_map<string , string > registerTab = {
        {"A", "0"}, {"X", "1"}, {"L", "2"}, {"B", "3"}, {"S", "4"}, {"T", "5"}, {"F", "6"}, {"PC", "8"}, {"SW", "9"}
    };

    unordered_map<string , int> symtab;

    int progStart = 0 , progLen = 0;

    cout << "Enter name of file\n";
    string str;
    cin >> str;

    ifstream ifile(str+".txt");
    ofstream ofile(str+"_Objectcode.txt");

    if (!ifile.is_open()) 
    {
        cerr << "Error opening input file!" << endl;
        return 1;
    }
    if (!ofile.is_open()) 
    {
        cerr << "Error opening output file!" << endl;
        return 1;
    }


    int val = progLen = firstPass (ifile , progStart , optab , symtab);

    ifile.clear();
    ifile.seekg(0);
    if(val == -1)
    {
         ifile.close();
        ofile.close();
        optab.clear();
        symtab.clear();
        cout << "Assembling terminated\nExiting program\n";
        return 1;
    }

    val = secondPass ( ifile , ofile , progStart ,progLen , optab , symtab );
    ifile.close();
    ofile.close();
    optab.clear();
    symtab.clear();
    registerTab.clear();
   
    if(val == -1)
    {
        cout << "Assembling terminated\nExiting program\n";
        return 1;
    }
    else
    {
        cout << "Assembling complete...\nThe Opcode generated is stored in ObjectCode.txt\n";
    }
    return 0;
}
