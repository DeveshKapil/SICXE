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
    ss >> opcode;
    if(!ss.eof()) ss >> operand;
    else operand = "";
}

int stoih(string s)
{
    int decimalValue;
    stringstream ss;
    ss << hex << s;
    ss >> decimalValue;

    return decimalValue;
}

void seperateReg(string &line , string &operand1 , string &operand2)
{
    operand1 = "" , operand2 = "";
    int n = line.find(',');
    if(n!=-1)
    {
        operand1 = line.substr(0 , n);
        operand2 = line.substr(n+1);
    }
    else operand1 = line;
}

void print(ofstream &ofile , int &locctr , int &start , string &objCode , int &count)
{
    ofile << "T" << intToHex(start , 6) << " " << intToHex(count , 2) << " " << objCode << endl ;
    objCode = "";
    start = locctr;
    count = 0;
}

int firstPass(ifstream &ifile , int &pstart , unordered_map<string , string> &optab , unordered_map<string , int> &symtab , unordered_map<string , string> &regtab)
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
            if (opcode == "END") break;
            else if(opcode == "WORD")
                locctr += 3;
            else if(opcode == "RESW")
                locctr += 3*stoi(operand);
            else if(opcode == "RESB")
                locctr += stoi(operand);
            else if(opcode == "BYTE")
                locctr += operand.length()-3;
            else if(opcode == "BASE");
            else if(opcode[0] == '+')
                locctr += 4;
            else if(regtab.find(opcode) != regtab.end())
                locctr += 2;
            else if(optab.find(opcode) != optab.end())
                    locctr += 3;
            else
            {
                cerr << "Opcode "<< opcode << " defined on line " << i << " was not found in the optab." << endl;
                return -1;
            }
        }
        i++;
    }
    return locctr - pstart;
}

int secondPass(ifstream &ifile ,ofstream &ofile , int &pstart , int &plen, unordered_map<string , string> &optab , unordered_map<string , int> &symtab , unordered_map<string , string> &regtab )
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
        else if (opcode == "RESW" || opcode == "RESB") // RESW  AND RESB
        {
            if(objCode.length()>0)
            {
                print(ofile , locctr , start , objCode , counter );
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
        }
        else if (opcode == "NOBASE")
        {
            base = -1;
        }
        else
        {
            if(objCode.length() == 0)
            {
                start = locctr;
            }
            if(opcode == "BYTE") // Byte
            {
                string temp = operand.substr(2,operand.length()-3);
                int len = temp.length();
                if(counter+len/2 > 30) 
                {
                    print(ofile , locctr , start , objCode , counter );
                }
                if(operand[0] == 'c')
                {
                    string temp1 = "";
                    for(char c : temp)
                    {
                        temp1 += intToHex(int(c) , 2);
                    }
                    temp = temp1;
                }
                objCode += temp;
                locctr += len/2;
                counter += len/2;
                
            }
            else if (opcode == "WORD") // Word
            {
                
                string temp ;
                if(operand[0] == 'X')
                {
                    temp = intToHex(stoi(operand.substr(2 , operand.length()-3)) , 3);
                }
                else if(operand[0] == 'C')
                {
                    temp = operand.substr(2 , operand.length()-3);
                    string temp1 = "";
                    for(char c : temp)
                    {
                        temp1 += intToHex(int(c) , 2);
                    }
                    for(int i = temp1.length()%3 ; i<3 ; i++)
                    {
                        temp1 = "00" + temp1;
                    }
                    temp = temp1;
                }
                if(counter+3 > 30) 
                {
                    print(ofile , locctr , start , objCode , counter );
                }
                objCode += temp;
                locctr += temp.length()/2;
                counter += temp.length()/2;
            }
            else
            {
                int val , disp;
                if(opcode[0] == '+') // Format four
                {
                    if(counter+4 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter );
                    }
                    string temp = optab[opcode.substr(1)];
                    if(operand.length() == 0)
                    {
                        temp += intToHex(0 , 6);
                    }
                    else if(operand[0] == '#')
                    {
                        disp = stoi(operand.substr(1));
                        temp = intToHex(stoih(temp)|1 , 2)+ "1" + intToHex(disp , 5);
                    }
                    else if(symtab.find(operand)!=symtab.end())
                    {
                        temp = intToHex(stoih(temp)|3 , 2) + "1" +  intToHex(symtab[operand] , 5);

                        qu.push("M " + intToHex(locctr+1 , 6));

                    }
                    else
                    {
                        cerr << "Symbol " << operand << " defined on line " << i << " was not found in the symtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    objCode += temp;
                    locctr += 4;
                    counter += 4;
                }
                else if (regtab.find(opcode) != regtab.end()) // Format two
                {
                     if(counter+2 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter);
                    }
                    string temp = regtab[opcode] , operand1 , operand2;
                    seperateReg(operand , operand1 , operand2);
                    if(regtab.find(operand1) == regtab.end() || (operand2.length() != 0 && regtab.find(operand2) == regtab.end()))
                    {
                        cerr << "Symbol " << operand << " defined on line " << i << " was not found in the regtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    temp += regtab[operand1] + (operand2.length() == 0 ? "0" : regtab[operand2]);
                    objCode+=temp;
                    locctr += 2;
                    counter += 2;
                }
                else{  // Format three
                    string temp = optab[opcode];
                    
                    val = 2;
                    if(operand.length() == 0)
                    {
                        temp += intToHex(0 , 4);
                    }
                    else if(operand[0] == '#')
                    {
                        try
                        {
                            disp = stoi(operand.substr(1));
                            temp = intToHex(stoih(temp)|1 , 2)+ "0" + intToHex(disp , 3);
                        }
                        catch(exception& e)
                        {
                            disp = symtab[operand.substr(1)] - locctr - 3;
                            temp = intToHex(stoih(temp)|1 , 2)+ "2" + intToHex(disp , 3);
                        }
                    }
                    else if(symtab.find(operand)!=symtab.end())
                    {
                        disp = symtab[operand] - locctr - 3;
                        if((disp < -2048 || disp > 2047) && base == -1) 
                        {
                            cerr << "Addresss out of range for format 3 addressing in line " << i << endl;
                            return -1;
                        }
                        else if(disp < -2048 || disp > 2047)
                        {
                            disp = base + symtab[operand];
                            val = 4;
                        }
                        else if(disp < 0)
                        {
                           disp = (!abs(disp))+1;
                           val = 2;
                        }
                        
                        temp = intToHex(stoih(temp)|3 , 2)+ intToHex(val , 1) + intToHex(disp , 3);
                    }
                    else
                    {
                        cerr << "Symbol " << operand << " defined on line " << i << " was not found in the symtab." << endl;
                        ofile.clear();
                        return -1;
                    }
                    if(counter+3 > 30)
                    {
                        print(ofile , locctr , start , objCode , counter );
                    }
                    cout << opcode  << "\t"  << optab[opcode] << "\t" << disp << "\t"  << "\t" << temp << endl;
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
        print(ofile , locctr , start , objCode , counter );
    }
    while(!qu.empty())
    {
        ofile << qu.front() << " 05" << endl;
        qu.pop();
    }
    ofile << "E" << intToHex(pstart , 6) << endl;
    return 0;
}


int main()
{
   unordered_map<string, string> optab = {
    {"LDA", "00"}, {"LDX", "04"}, {"LDL", "08"}, {"STA", "0C"}, {"STX", "10"}, {"STL", "14"},
    {"ADD", "18"}, {"SUB", "1C"}, {"MUL", "20"}, {"DIV", "24"}, {"COMP", "28"}, {"TIX", "2C"},
    {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"}, {"J", "3C"}, {"AND", "40"}, {"OR", "44"},
    {"JSUB", "48"}, {"RSUB", "4C"}, {"LDCH", "50"}, {"STCH", "54"}, {"ADDF", "58"}, {"SUBF", "5C"},
    {"MULF", "60"}, {"DIVF", "64"}, {"LDB", "68"}, {"LDS", "6C"}, {"LDF", "70"}, {"LDT", "74"},
    {"STB", "78"}, {"STS", "7C"}, {"STF", "80"}, {"STT", "84"}, {"COMPF", "88"},
    {"FLOAT", "C0"}, {"FIX", "C4"}, {"NORM", "C8"}, {"LPS", "D0"},
    {"STSW", "E8"}, {"RD", "D8"}, {"WD", "DC"}, {"TD", "E0"}, {"SSK", "EC"}, {"STI", "D4"}
    };
    unordered_map<string, string> registerTab = {
    {"A", "0"}, {"X", "1"}, {"L", "2"}, {"B", "3"}, {"S", "4"}, {"T", "5"}, {"F", "6"}, {"PC", "8"}, {"SW", "9"},
    {"ADDR", "90"}, {"SUBR", "94"}, {"MULR", "98"}, {"DIVR", "9C"}, {"TIXR", "B8"}, {"CLEAR", "B4"},
    {"SHIFTL", "A4"}, {"SHIFTR", "A8"}
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


    int val = progLen = firstPass (ifile , progStart , optab , symtab , registerTab);

    ifile.clear();
    ifile.seekg(0);
    if(val == -1)
    {
        ifile.close();
        ofile.close();
        optab.clear();
        symtab.clear();
        registerTab.clear();
        cout << "Assembling terminated\nExiting program\n";
        return 1;
    }

    val = secondPass ( ifile , ofile , progStart ,progLen , optab , symtab , registerTab);
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
