#include<iostream>
#include<unordered_map>
#include<string>
#include<iomanip>
#include<fstream>
#include<sstream>

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

void print(ofstream &ofile , int &locctr , int &start , string &objCode , int &count)
{
    ofile << "T" << intToHex(start , 6) << " " << intToHex(count , 2) << " " << objCode << endl;
    objCode = "";
    start = locctr;
    count = 0;
}

int firstPass(ifstream &ifile , int &pstart , unordered_map<string , string> &optab , unordered_map<string , int> &symtab)
{
    string line , label , opcode , operand;
    int locctr = 0;

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
                symtab[label] = locctr;

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
                locctr += 3;
        }
    }
    return locctr - pstart;
}

int secondPass(ifstream &ifile ,ofstream &ofile , int &pstart , int &plen, unordered_map<string , string> &optab , unordered_map<string , int> &symtab)
{
    string line, label, opcode, operand , objCode;
    int locctr = pstart , i=1 , counter = 0;
    int start = locctr;


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
                print(ofile , locctr , start , objCode , counter);
            }
            if (opcode == "RESW")
            {
                locctr += 3*stoi(operand);
            }
            locctr += stoi(operand);
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
                    print(ofile , locctr , start , objCode , counter);
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
                    print(ofile , locctr , start , objCode , counter);
                }
                objCode += temp;
                locctr += 3;
                counter += 3;
            }

            else if(opcode[0] == '+')
            {
                string temp = optab[opcode.substr(1)];
                stringstream ss(operand);
                string s1 , s2;
                getline(ss , s1 , ',');
                getline(ss , s2);
                if(s2.length() != 0)
                    temp += 
                else
                    temp += intToHex(symtab[operand] , 5);

                locctr += 4;
                counter += 4;
            }
            
            else
            {
                if(optab.find(opcode)!=optab.end())
                {
                    string temp = optab[opcode];
                    if(symtab.find(operand)!=symtab.end())
                        temp += intToHex(symtab[operand] , 4);
                    else
                    {
                        cerr << "Operand defined on line " << i << " " << operand << " was not found in the symtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    if(counter+3 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter);
                    }
                    objCode += temp;
                    locctr += 3;
                    counter += 3;
                }
                else
                {
                    cerr << "Opcode defined on line " << i <<" " << opcode << " was not found in the optab." << endl;
                    ofile.clear();
                    return -1;
                }
            }
        }
        
        i++;
    }
    if(objCode.length()>0)
    {
        print(ofile , locctr , start , objCode , counter);
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


    progLen = firstPass (ifile , progStart , optab , symtab);

    ifile.clear();
    ifile.seekg(0);

    int val = secondPass ( ifile , ofile , progStart ,progLen , optab , symtab );
    ifile.close();
    ofile.close();
    optab.clear();
    symtab.clear();
   
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
