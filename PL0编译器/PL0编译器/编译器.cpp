#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<fstream>
#include<string>
#include<iomanip>
#include<algorithm>
#include<vector>
#include<windows.h>
#include<stack>
using namespace std;
const int INF = 1e9 + 7;								//�����������ֵ
const int MAXN = 1e5 + 5;								//������ű��������ֵ


/**************************************��ر������������****************************************/
struct Word {
	string word_content;						//��������
	string word_type;							//��������
	int row;									//������������
	Word() {}
	Word(string wc, int row, string wt) {
		word_content = wc;
		this->row = row;
		word_type = wt;
	}
	void printWord() {
		//��ӡʶ��ĵ��ʽ��
		cout << setw(10) << left << word_content << "     " << setw(10) << word_type << "       " << row << endl;
	}
	bool operator==(const Word& word) {
		return this->word_content == word.word_content;
	}
};
vector<Word>words;										//����洢�ʷ��������Ľ������
struct Error {
	string error_content;						//�����������
	string error_type;							//�����������
	int row;									//���������������
	Error(string ec, int row, string et) {
		this->error_content = ec;
		this->error_type = et;
		this->row = row;

	}
	void print_error() {
		cout << "Error Location in row[" << this->row << "]  ";
		cout << error_type << " : " << error_content << "!" << endl;
	}
};
vector<Error>errors;									//����洢����Ľṹ��

struct entry {
	string name;							//������
	string kind;							//���������
	int level;								//�������ڲ�Ĵ�С
	int offset;								//�������Ե�ַ(�Զ�����)
	int val;								//��Ϊ�������߳�������Ҫ��ֵ
	int previous;							//��������ͨ������ȥ���ң����ֵΪ�㣬��ʾ�ù�����û��ָ�����ݣ��Զ�����)
	int para_num;							//��������Ӧ��ֵ����ʾ�ù��̶�Ӧ�Ĳ�������
	entry() {}
	entry(string n, string k, int l, int o, int v, int p, int pa) {
		this->name = n;
		this->kind = k;
		this->level = l;
		this->offset = o;
		this->val = v;
		this->previous = p;
		this->para_num = pa;
	}
};
vector<entry>entries(MAXN);								//ջʽ���ű�	
vector<int>entry_display;								//�����Ӧ����ʾ���ű�

struct code {
	string op_name;					//������
	int level;						//����
	int op_num;						//������
	code() {}
	code(string oname, int l, int onum) {
		this->op_name = oname;
		this->op_num = onum;
		this->level = l;
	}
	void printCode(int width) {
		cout << setw(width) << left << this->op_name << setw(width) << left << this->level << setw(width) << left << this->op_num << endl;
	}
};
vector<code>codes;										//��������ַ����

unordered_map<int, string>all_errors;					//�������ļ�ֵ�Լ���
unordered_map<int, string>all_errors_tp;				//����������ͱ�

unordered_set <string>reserved;							//����ؼ��ֵļ���
unordered_set <string>symbol;							//������ŵļ���

vector<int>act_stack(MAXN);								//������¼ջ
stack<int>data_stack;									//��������ջ

int act_top = 0;										//����ջջ��ָ��
int sp = 0;												//���嵱ǰ���̵���ʼָ��
int entry_top = 0;										//����ջʽ���ű��ջ��
int cru_level = 0;										//Ƕ�ײ���

int point = 0;											//����ȫ��ָ��
int row = 0;											//���嵱ǰʶ����
int word_point = 0;										//����ÿһ���﷨��λ��ָ��
Word tword;												//ȫ�ֵĵ��ʻ���

/*******************************************���庯������**************************************************/
int changeNum(string);
void initErrors();
void trim(string&);
bool isDigit(char);
bool isLetter(char);
bool isSymbol(char);
void readProperitesToSet(string, string);
void checkIsNumber(string);
void reservedOrIdentifier(string);
void checkIsWord(string);
void checkIsSymbol(string);
void checkOthers(string);
void startAnalysis(string);
void lexicalAnalysis(string);
bool synchronous(int);
void pushError(int id, int type = 1, string content = "");
Word getNextWord();
void errorWhile(int, int);
void constAnalyzer();
void condeclAnalyzer();
void vardeclAnalyzer();
void procedureAnalyzer();
void factorAnalyzer();
void termAnalyzer();
void expAnalyzer();
void lexpAnalyzer();
void expRecursion(string, int&);
void expPaAnalyzer(string, int&);
void idRecursion(string);
void idPaAnalyzer(string);
void stateAnalyzer();
void stateWhile();
void bodyAnalyzer();
void blockAnalyzer();
void progAnalyzer();
void syntaxAnalyzer();
void errorWhile(int, int);
void addEntry(string name, string kind, int level, int val = INF);
void delEntry();
pair<int, int> searchEntry(string);
void updateEntry(string, int);
int judgeDefined(string);
int emitCode(string op_name, int level = 0, int op_num = INF);
void backPatch(int, int);
int oprType(string);
int getSize();
int findVal(int, int);
void setVal(int, int, int);
pair<int, int>getTwoTop();
int getStaticChain(int);
int codeAnalyzer(int);
void interpreter();


/*******************************************���ߺ���**************************************************/
int changeNum(string str) {
	int len = str.size();
	int j = 1;
	int ans = 0;
	for (int i = len - 1; i >= 0; i--) {
		ans += j * (str[i] - '0' + 0);
		j *= 10;
	}
	return ans;
}
void initErrors() {
	//������Ӧ�Ĵ����ʼ�������﷨����������ǰ�����д���������
	all_errors[1] = "Identifier Cannot Start With A Number";			//��ʶ�����ֿ�ͷ
	all_errors[2] = "Assignment Symbol Is Missing ��=��.";				//��ֵ���Ŵ���
	all_errors[3] = "Unknown Lexical Error";							//δ֪�Ĵʷ�����

	all_errors[4] = "Invalid Or Missing RESERVED 'program'";			//program����
	all_errors[5] = "Identifier Expected";								//ȱ�ٱ�ʶ��
	all_errors[6] = "';' Symbol Expected";								//ȱ��';'
	all_errors[7] = "':=' Symbol Expected";								//ȱ��'��='
	all_errors[8] = "Invalid Or Missing INTEGER";						//ȱ������
	all_errors[9] = "',' Symbol Expected";								//ȱ��','
	all_errors[10] = "Unexpected Identifier or Symbol";					//δ����ķ���
	all_errors[11] = "'(' Symbol Expected";								//ȱ��'('
	all_errors[12] = "Argument Is Missing";								//ȱ�ٲ���
	all_errors[13] = "')' Symbol Expected";								//ȱ��')'
	all_errors[14] = "Invalid Or Missing RESERVED 'begin'";				//ȱ��begin
	all_errors[15] = "LOP Symbol Expected";								//ȱ��lop
	all_errors[16] = "Invalid Or Missing RESERVED 'then'";				//ȱ��then
	all_errors[17] = "Invalid Or Missing RESERVED 'do'";				//ȱ��do
	all_errors[18] = "Has Already Defined";								//�ض���
	all_errors[19] = "Invalid Or Missing RESERVED 'end'";				//ȱ��end
	all_errors[20] = "Too many parameters passed in";					//�����������
	all_errors[21] = "Too few parameters passed in";					//�������̫��
	all_errors[22] = "Undefined Identifier";							//��ʶ��δ����




	all_errors_tp[1] = "Syntax Error";									//�﷨����
	all_errors_tp[2] = "Lexical Error";									//�ʷ�����
	all_errors_tp[3] = "Semantic Error";								//�������
}
void pushError(int id, int type, string content) {
	string err;
	if (type == 3) {
		err = '\'' + content + '\'' + ' ' + all_errors[id];
	}
	else {
		err = all_errors[id];
	}
	errors.push_back(Error(err, tword.row, all_errors_tp[type]));
	errors[errors.size() - 1].print_error();
	exit(0);											//�д���ֱ����ֹ����
}

/********************************************�ʷ�����**************************************************/
void trim(string& s)
{
	//ȥ�ַ������ߵĿո���Ʊ��
	if (s.empty())
	{
		return;
	}
	s.erase(0, s.find_first_not_of(" \t"));
	s.erase(s.find_last_not_of(" \t") + 1);

}
bool isDigit(char ch) {
	//ʶ���ַ��Ƿ�Ϊ����
	if (ch >= '0' && ch <= '9') {
		return 1;
	}
	return 0;
}
bool isLetter(char ch) {
	//ʶ���Ƿ����ַ�
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
		return 1;
	}
	return 0;
}
bool isSymbol(char ch) {
	//ʶ���Ƿ�Ϊ�����е��ַ�
	for (auto i : symbol) {
		if ((i.find(ch) != string::npos)) {
			return 1;
		}
	}
	return 0;
}
void readProperitesToSet(string url1, string url2) {
	//�����ȡ�����ļ��ķ�����װ�����Ӧ������
	fstream file;
	file.open(url1, ios::in);
	if (file.fail()) {
		cout << url1 << " �ļ���ʧ�ܣ�" << endl;
		exit(0);
	}
	string temp;
	while (getline(file, temp)) {
		trim(temp);
		reserved.insert(temp);
	}
	file.close();

	file.open(url2, ios::in);
	if (file.fail()) {
		cout << url2 << " �ļ���ʧ�ܣ�" << endl;
		exit(0);
	}
	while (getline(file, temp)) {
		trim(temp);
		symbol.insert(temp);
	}
	file.close();
}
void checkIsNumber(string str) {
	//��鵱ǰ�ֶ��Ƿ�Ϊ����
	string token;
	token.push_back(str[point++]);
	char ch;
	for (; point < str.size(); point++) {
		ch = str[point];
		if (isdigit(ch)) {
			token.push_back(ch);
			continue;
		}
		if (ch == ' ' || isSymbol(ch)) {
			point--;
			break;
		}
		else {
			//����ʶ�������֮�󣬱�����󣬽�ָ����ת���ո�λ��
			pushError(1, 2);
			while (point + 1 < str.size() && str[point + 1] != ' ') {
				point++;
			}
			return;
		}
	}
	//���ֱ�ʾ��ǰtoken�Ѿ�ʶ���������ʶ��ɹ�
	words.push_back(Word(token, row, "INTEGER"));
}
void reservedOrIdentifier(string token) {
	if (reserved.find(token) != reserved.end()) {
		words.push_back(Word(token, row, "RESERVED"));
	}
	else {
		words.push_back(Word(token, row, "IDENTIFIER"));
	}
}
void checkIsWord(string str) {
	//��鵱ǰ�ֶ��Ƿ�Ϊ�Ϸ���ʶ��
	string token;
	token.push_back(str[point++]);
	char ch;
	for (; point < str.size(); point++) {
		ch = str[point];
		if (!(isLetter(ch) || isDigit(ch))) {
			point--;
			break;
		}
		else {
			token.push_back(ch);
		}
	}
	reservedOrIdentifier(token);
}
void checkIsSymbol(string str) {
	//��鵱ǰ�ֶ��Ƿ�Ϊ�Ϸ�����
	string token;
	char ch = str[point];
	token.push_back(str[point++]);
	//��֧�ж�
	switch (ch) {
	case ';':
		words.push_back(Word(token, row, "COLON"));
		point--;
		break;
	case ',':
		words.push_back(Word(token, row, "COMMA"));
		point--;
		break;
	case '=':
		words.push_back(Word(token, row, "EQUAL"));
		point--;
		break;
	case '+':
		words.push_back(Word(token, row, "PLUS"));
		point--;
		break;
	case '-':
		words.push_back(Word(token, row, "SUB"));
		point--;
		break;
	case '/':
		words.push_back(Word(token, row, "DIV"));
		point--;
		break;
	case '(':
		words.push_back(Word(token, row, "LBRACKET"));
		point--;
		break;
	case ')':
		words.push_back(Word(token, row, "RBRACKET"));
		point--;
		break;
	case '.':
		words.push_back(Word(token, row, "POINT"));
		point--;
		break;
	case '>':
		if (point < str.size() && str[point] == '=') {
			words.push_back(Word(token, row, "GREATERANDEQUAL"));
		}
		else {
			words.push_back(Word(token, row, "GREATER"));
			point--;
		}
		break;
	case '<':
		if (point < str.size() && str[point] == '=') {
			token.push_back(str[point]);
			words.push_back(Word(token, row, "GREATERANDEQUAL"));
		}
		else if (point < str.size() && str[point] == '>') {
			token.push_back(str[point]);
			words.push_back(Word(token, row, "NOTEQUAL"));
		}
		else {
			words.push_back(Word(token, row, "LESS"));
			point--;
		}
		break;
	case ':':
		if (point < str.size() && str[point] == '=') {
			token.push_back(str[point]);
			words.push_back(Word(token, row, "ASSIGNMENT"));
		}
		else {
			pushError(2, 2);
			point--;
		}
		break;
	case '*':
		if (point < str.size() && str[point] == '*') {
			str.push_back(str[point]);
			words.push_back(Word(token, row, "EXP"));
		}
		else {
			words.push_back(Word(token, row, "MULTIPLI"));
			point--;
		}
		break;
	default:break;
	}
}
void checkOthers(string str) {
	string token;
	token.push_back(str[point++]);
	char ch;
	for (; point < str.size(); point++) {
		//�ҵ��ո�
		ch = str[point];
		if (ch == ' ') {
			return;
		}
		else {
			token.push_back(ch);
		}
	}

	//��Ӵʷ�����
	pushError(3, 2, token);
}
void startAnalysis(string str) {
	if (str.size() == 0) {
		//�����ַ���Ϊ��ֱ�ӷ���
		return;
	}
	else {
		char ch;
		//ȡ����һ���ַ�
		for (; point < str.size(); point++) {
			ch = str[point];
			if (isDigit(ch)) {
				//��ͷ�����֣��жϸ��ֶ��Ƿ�����
				checkIsNumber(str);
			}
			else if (isLetter(ch)) {
				checkIsWord(str);
			}
			else if (ch == ' ') {
				continue;
			}
			else if (isSymbol(ch)) {
				checkIsSymbol(str);
			}
			else {
				checkOthers(str);
			}
		}
	}
}
void lexicalAnalysis(string url) {
	//��ȡ�����ļ������з���
	fstream file;
	file.open(url, ios::in);
	if (file.fail()) {
		cout << url << "�ļ���ʧ�ܣ�" << endl;
		exit(0);
	}
	string temp;
	while (getline(file, temp)) {
		trim(temp);
		point = 0;
		row++;
		startAnalysis(temp);
	}
	file.close();
}


/********************************************������﷨����**********************************************/
bool synchronous(int pi) {
	switch (pi) {
	case 1: return tword.word_content != ";" && tword.word_content != "const" && tword.word_content != "var" && tword.word_content != "procedure" && tword.word_content != "begin";
	case 2: return tword.word_content != "const" && tword.word_content != "var" && tword.word_content != "procedure" && tword.word_content != "begin";
	case 3: return tword.word_content != ":=" && tword.word_type != "INTEGER" && tword.word_content != "," && tword.word_content != ";";
	case 4: return tword.word_type != "INTEGER" && tword.word_content != "," && tword.word_content != ";";
	case 5: return tword.word_content != "," && tword.word_content != ";";
	case 6: return tword.word_content != ";";
	case 7: return tword.word_content != "(" && tword.word_content != ";";
	case 8: return tword.word_type != "IDENTIFIER" && tword.word_content != ")" && tword.word_content != ";";
	case 9: return tword.word_content != "," && tword.word_content != ")" && tword.word_content != ";";
	case 10: return tword.word_content != "begin" && tword.word_content != "while" && tword.word_content != "call" && tword.word_content != "read" && tword.word_content != "write" && tword.word_content != "if" && tword.word_type != "IDENTIFIER";
	case 11: return tword.word_content != "+" && tword.word_content != "-" && tword.word_content != "end" && tword.word_type != "INTEGER" && tword.word_type != "IDENTIFIER" && tword.word_content != ";";
	case 12: return tword.word_content != "=" && tword.word_content != "<>" && tword.word_content != "<" && tword.word_content != "<=" && tword.word_content != ">" && tword.word_content != ">=";
	case 13: return tword.word_content != "then" && tword.word_content != "begin" && tword.word_content != "while" && tword.word_content != "call" && tword.word_content != "read" && tword.word_content != "write" && tword.word_content != "if" && tword.word_type != "IDENTIFIER";
	case 14: return tword.word_content != "begin" && tword.word_content != "while" && tword.word_content != "call" && tword.word_content != "read" && tword.word_content != "write" && tword.word_content != "if" && tword.word_type != "IDENTIFIER" && tword.word_content != "(";
	case 15: return tword.word_content != ")" && tword.word_content != "+" && tword.word_content != "-" && tword.word_type != "IDENTIFIER" && tword.word_content != "(" && tword.word_type != "INTEGER";
	case 16: return tword.word_content != ";" && tword.word_content != "end";
	default:return 0;
	}
}
void errorWhile(int error_id, int syn_id) {
	//error_id�Ǳ������ݵı�ţ�syn_id��ͬ�����жϷ���
	pushError(error_id);
	while (synchronous(syn_id)) {
		tword = getNextWord();
	}
	word_point--;
}
Word getNextWord() {
	//������һ���﷨��λ
	if (word_point >= words.size()) {
		//����Ѿ�û�е��ʿ��Զ�ȡ����ֱ�ӷ��ء�
		exit(0);
	}
	if (word_point < 0) {
		word_point = 0;
	}
	tword = words[word_point++];
	return tword;
}
void constAnalyzer() {
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 3);														//const���ʽȱ��id������ͬ��
	}
	else {
		if (searchEntry(tword.word_content).first != -1) {						//������Ҫ�жϸñ�ʶ���Ƿ��ѱ����壬����ڴ����г��ֹ��ͱ����ض���
			pushError(18, 3, tword.word_content);
		}
		else {
			addEntry(tword.word_content, "const", cru_level);
		}
	}

	if (getNextWord().word_content != ":=") {
		errorWhile(7, 4);														//const���ʽ':='ȱ�ٻ��߲���ȷ������ͬ��
	}
	if (getNextWord().word_type != "INTEGER") {
		errorWhile(9, 5);														//const���ʽȱ��int������ͬ��
	}
	else {
		//�ѱ�����Ӧ��ֵ����
		entries[entry_top - 1].val = changeNum(tword.word_content);
	}

	if (getNextWord().word_content == ",") {
		constAnalyzer();														//ʶ����һ��const,�ݹ����
	}
	else if (tword.word_content == ";") {
		return;																	//ʶ��';'��ֱ�ӽ���
	}
	else if (tword.word_type == "IDENTIFIER") {
		pushError(9);															//ȱ�ٷָ�����',',��������
		word_point--;
		constAnalyzer();
	}
	else {																		//�������ţ�����ͬ��
		errorWhile(10, 6);
	}
}
void condeclAnalyzer() {
	constAnalyzer();
}
void vardeclAnalyzer() {
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 5);													//ȱ��id������ͬ��
	}
	else {
		if (judgeDefined(tword.word_content) == -1) {						//�жϱ����Ƿ��ض���
			addEntry(tword.word_content, "var", cru_level);
		}
		else {
			pushError(18, 3, tword.word_content);
		}
	}
	tword = getNextWord();
	if (tword.word_content == ",") {
		vardeclAnalyzer();
	}
	else if (tword.word_content == ";") {									//var��������
		return;
	}
	else if (tword.word_type == "IDENTIFIER") {								//ȱ��','
		pushError(9);
		word_point--;
		vardeclAnalyzer();
	}
	else {
		errorWhile(10, 6);
	}
}
void procedureAnalyzer() {
	cru_level++;																//Ƕ�ײ��һ
	if (getNextWord().word_type != "IDENTIFIER") {								//ȱ��id������ͬ��
		errorWhile(5, 7);
	}
	else {
		if (judgeDefined(tword.word_content) == -1) {							//�жϸù������Ƿ��Ѿ�������
			addEntry(tword.word_content, "procedure", cru_level);
			//�����м���룺����������ת���
			int back_loc = emitCode("JMP");
			//���浱ǰ���̶�Ӧ����λ��
			entries[entry_top - 1].val = back_loc;
		}
		else {
			pushError(18, 3, tword.word_content);
		}
	}
	idPaAnalyzer("procedure");													//�����б�ѭ������
	if (getNextWord().word_content != ";") {									//����'��'
		errorWhile(6, 13);
	}
	blockAnalyzer();															//����block
	//�����м���룺�ù��̽������ͷŶ�Ӧ��ջ�ռ�
	emitCode("OPR", 0, 0);
	delEntry();																	//������procedure��ɾ�����̶�Ӧ�ı��
	cru_level--;																//�����꣬��ǰ��Ҫ��һ

	tword = getNextWord();
	if (tword.word_content == ";") {											//�ж���һ�������ǲ���procedure
		tword = getNextWord();
		procedureAnalyzer();
	}
	else {
		if (tword.word_content == "procedure") {
			pushError(6);
			procedureAnalyzer();
		}
		word_point--;
	}
}
void factorAnalyzer() {
	tword = getNextWord();
	if (tword.word_type == "IDENTIFIER") {
		pair<int, int> loc = searchEntry(tword.word_content);
		if (loc.first == -1) {												//�ж��Ƿ񱻶���
			pushError(22, 3, tword.word_content);							//��ʶ��δ����
		}
		else {
			if (entries[loc.first].kind == "const") {
				//�����м���룺����������ջ
				emitCode("LIT", 0, entries[loc.first].val);
			}
			else {
				//�����м���룺�βλ��߱�������

				emitCode("LOD", loc.second, entries[loc.first].offset + 2);
			}
		}
	}
	else if (tword.word_type == "INTEGER") {
		//�����м���룺����������ջ
		emitCode("LIT", 0, changeNum(tword.word_content));
	}
	else if (tword.word_content == "(") {								//(<exp>)
		expAnalyzer();
		if (getNextWord().word_content != ")") {
			errorWhile(13, 6);											//����ȱ��'��'��ͬ��
		}
	}
	else {
		errorWhile(10, 6);												//�����������������
	}
}
void termAnalyzer() {
	factorAnalyzer();
	tword = getNextWord();
	if (tword.word_content == "*" || tword.word_content == "/") {
		int opr = oprType(tword.word_content);
		termAnalyzer();
		//�����м���룺��������
		emitCode("OPR", 0, opr);
	}
	else {
		word_point--;
	}
}
void expAnalyzer() {
	tword = getNextWord();
	bool flag = 0;
	if (tword.word_content == "+" || tword.word_content == "-") {
		if (tword.word_content == "-") {
			//��������ָ���0��������ջ
			emitCode("OPR", 0, oprType("0"));
			flag = 1;
		}
		tword = getNextWord();										//���Ե�'+'��'-'
	}
	word_point--;
	termAnalyzer();													//����term
	if (flag == 1) {
		//���ɼ���ָ��
		emitCode("OPR", 0, oprType("-"));
	}
	tword = getNextWord();
	if (tword.word_content == "+" || tword.word_content == "-") {	//�жϽ������Ƿ���
		int opr = oprType(tword.word_content);
		termAnalyzer();
		//�����м���룺�Ӽ�����
		emitCode("OPR", 0, opr);
	}
	else {
		word_point--;
	}
}
void lexpAnalyzer() {
	tword = getNextWord();
	if (tword.word_content == "odd") {
		expAnalyzer();
		emitCode("OPR", 0, oprType("odd"));
	}
	else if (!synchronous(11)) {											//�����<exp>��first
		word_point--;
		expAnalyzer();													//����<exp>
		tword = getNextWord();
		if (!synchronous(12)) {											//�ж��Ƿ�Ϊ<lop>
			int opr = oprType(tword.word_content);
			expAnalyzer();												//��lop���ٽ���<exp>����
			//�����м���룺opr����
			emitCode("OPR", 0, opr);
		}
		else {
			errorWhile(15, 13);											//���򱨴�ȱ��<lop>
		}
	}
	else {
		errorWhile(10, 13);												//����ͬ��
	}
}
void expRecursion(string select, int& off) {
	expAnalyzer();												//����<exp>
	if (select == "call") {
		//�����м���룺��������											
		emitCode("ARG", 0, off);
		off++;
	}
	else {
		//�����м���룺wrt���ջ��
		emitCode("WRT", 0, 0);
	}


	tword = getNextWord();										//��������ķ���
	if (tword.word_content == ",") {
		expRecursion(select, off);
	}
	else if (tword.word_content == ")") {						//�˳�
		return;
	}
	else if (tword.word_content == ";") {						//����ȱ��')'������
		pushError(13);
		word_point--;
		return;
	}
	else if (!synchronous(11)) {								//ȱ��','
		pushError(9);
		word_point--;
		expRecursion(select, off);
	}
	else {
		errorWhile(10, 6);
	}
}
void expPaAnalyzer(string select, int& off) {
	if (getNextWord().word_content != "(") {							//����'('
		errorWhile(11, 15);
	}
	tword = getNextWord();
	if (tword.word_content == ")") {
		if (select == "write") {
			pushError(12);												//ȱ��<exp>
		}
	}
	else if (tword.word_content == ";") {
		if (select == "write") {
			pushError(12);												//ȱ��<exp>
		}
		else {
			pushError(13);												//ȱ��')'
		}
		word_point--;
	}
	else {																//�����������exp
		word_point--;
		expRecursion(select, off);
	}
}
void idRecursion(string select) {
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 9);														//ȱ��id������ͬ��
	}
	else {
		if (select == "procedure") {											//�ж��Ƿ��ʶ���Ѿ�������
			if (judgeDefined(tword.word_content) == -1) {
				addEntry(tword.word_content, "parameter", cru_level);
			}
			else {
				pushError(18, 3, tword.word_content);
			}

		}
		else {																	//�����read����ô���ɴ���
			pair<int, int>loc = searchEntry(tword.word_content);
			if (loc.first == -1) {
				pushError(22, 3, tword.word_content);
			}
			else {
				//�����м���룺red��ȡ����
				emitCode("RED", 0, 0);
				//�����м���룺sto��ȡջ��Ԫ�ص�ָ��λ��
				emitCode("STO", loc.second, entries[loc.first].offset + 2);
			}
		}
	}
	tword = getNextWord();
	if (tword.word_content == ",") {
		idRecursion(select);
	}
	else if (tword.word_content == ")") {
		return;
	}
	else if (tword.word_content == ";") {
		pushError(13);
		word_point--;
		return;
	}
	else if (tword.word_type == "IDENTIFIER") {
		pushError(9);
		word_point--;
		idRecursion(select);
	}
	else {
		errorWhile(10, 6);
	}

}
void idPaAnalyzer(string select) {
	//Ϊ�˴��븴�üӵ��жϲ�����selectΪ'procedure'��ʾ�����б����Ϊ�գ�'read'��ʾ����Ϊ��
	if (getNextWord().word_content != "(") {								//ȱ��'('������ͬ��				
		errorWhile(11, 8);
	}
	tword = getNextWord();
	if (tword.word_content == ")") {
		if (select == "read") {
			pushError(12);
		}
	}
	else if (tword.word_content == ";") {
		if (select == "read") {
			pushError(12);
		}
		else {
			pushError(13);
		}
		word_point--;
	}
	else {
		word_point--;
		idRecursion(select);
	}
}
void stateWhile() {
	while (1) {
		tword = getNextWord();
		if (tword.word_content == ";") {
			stateAnalyzer();
		}
		else if (tword.word_content == "end") {
			break;
		}
		else if (synchronous(14)) {
			pushError(19);									//����ȱ��end
			word_point--;
			break;
		}
		else {
			pushError(6);									//����ȱ��';'
			word_point--;
			stateAnalyzer();
		}
	}
}
void stateAnalyzer() {
	tword = getNextWord();
	if (tword.word_content == "if") {
		lexpAnalyzer();													//����<lexp>	
		//�����м���룺�ж�ջ���Ƿ�Ϊ�棨��Ҫ����ٳ���
		int back_loc_false = emitCode("JPC");
		if (getNextWord().word_content != "then") {						//����'then'
			errorWhile(16, 10);
		}
		stateAnalyzer();
		if (getNextWord().word_content == "else") {						//����'else'
			//�����м���룺�ж�ջ���Ƿ�Ϊ�棨��Ҫ��������
			int back_loc_true = emitCode("JMP");
			//����ٳ���
			backPatch(back_loc_false, codes.size());
			stateAnalyzer();
			//���������
			backPatch(back_loc_true, codes.size());
		}
		else {
			//����ٳ���
			backPatch(back_loc_false, codes.size());
		}
	}
	else if (tword.word_content == "while") {
		//��¼��������ת�Ļ����ַ
		int return_loc = codes.size();
		lexpAnalyzer();															//����<lexp>	
		//�����м���룺��������ת�������
		int back_loc = emitCode("JPC");
		if (getNextWord().word_content != "do") {
			errorWhile(17, 10);													//����'do',û�б���ͬ��
		}
		stateAnalyzer();														//����<statement>
		//while����Ҫ��һ����������ת���
		int back_loc_return = emitCode("JMP");
		backPatch(back_loc_return, return_loc);
		//����ѭ������
		backPatch(back_loc, codes.size());
	}
	else if (tword.word_content == "call") {
		pair<int, int> id_loc;
		int pa_off = 3;
		if (getNextWord().word_type != "IDENTIFIER") {							//����<id>
			errorWhile(5, 14);
		}
		else {
			id_loc = searchEntry(tword.word_content);
			if (id_loc.first == -1) {											//�жϹ����Ƿ񱻶���
				pushError(22, 3, tword.word_content);
			}
		}

		expPaAnalyzer("call", pa_off);											//����<exp>����

		if (pa_off - 3 > entries[id_loc.first].para_num) {						//�жϲ����Ƿ����
			pushError(20, 3, entries[id_loc.first].name);
		}
		else if (pa_off - 3 < entries[id_loc.first].para_num) {
			pushError(21, 3, entries[id_loc.first].name);
		}
		else {
			//�����м���룺��������
			emitCode("CAL", id_loc.second, entries[id_loc.first].val);
		}
	}
	else if (tword.word_content == "read") {
		idPaAnalyzer("read");													//����id����������һ��
	}
	else if (tword.word_content == "write") {
		int pa_num = 3;
		expPaAnalyzer("write", pa_num);											//�������ʽ����������Ϊһ��
		//�����м���룺����
		emitCode("OPR", 0, oprType("#"));
	}
	else if (tword.word_type == "IDENTIFIER") {
		pair<int, int>loc = searchEntry(tword.word_content);
		if (loc.first == -1) {
			pushError(22, 3, tword.word_content);
		}

		if (getNextWord().word_content != ":=") {								//ȱ��':='������ͬ��
			errorWhile(7, 16);
		}
		tword = getNextWord();
		word_point--;
		if (!synchronous(15)) {
			expAnalyzer();
		}
		//�����м����
		emitCode("STO", loc.second, entries[loc.first].offset + 2);
	}
	else if (tword.word_content == "begin") {
		stateAnalyzer();
		stateWhile();
	}
	else {																		//��������ͱ���ͬ��
		pushError(10);
		while (synchronous(15)) {
			tword = getNextWord();
		}
		word_point--;
	}
}
void bodyAnalyzer() {
	tword = getNextWord();
	if (synchronous(10)) {														//����δ֪�����ҵ�ͬ��
		pushError(10);
		while (synchronous(10)) {
			tword = getNextWord();
		}
	}
	if (tword.word_content != "begin") {
		word_point--;
	}
	//�������JMPָ��
	int code_loc = entries[entry_display.back() - 1].val;
	codes[code_loc].op_num = codes.size();

	//�����м���룺�ڻ��¼�п��ٿռ�
	int size = getSize();
	emitCode("INT", 0, size);

	stateAnalyzer();														//����<statement>
	stateWhile();															//����{;<statement>}
}
void blockAnalyzer() {
	if (getNextWord().word_content == "const") {
		condeclAnalyzer();													//ת������"condecl"
	}
	else {
		word_point--;
	}
	if (getNextWord().word_content == "var") {
		vardeclAnalyzer();													//ת������"vardecl"
	}
	else {
		word_point--;
	}
	if (getNextWord().word_content == "procedure") {
		procedureAnalyzer();												//ת������"procedure"
	}
	else {
		word_point--;
	}

	if (getNextWord().word_content != "begin") {							//����begin
		errorWhile(14, 2);													//�ж��Ƿ�������仹��<block>��first
		if (tword.word_content != "begin") {
			blockAnalyzer();
			return;
		}
	}
	bodyAnalyzer();
}
void progAnalyzer() {
	if (getNextWord().word_content != "program") {
		errorWhile(4, 1);											//program���󣬱���ͬ��
		word_point--;												//����ͬ�������һ������Ϊƴ���program�ᱻ����id������judge�в�����id���ж�
	}
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 1);											//id���󣬱���ͬ��
	}
	else {
		addEntry(tword.word_content, "procedure", cru_level);		//ʶ��ɹ��������ű�
		int back_loc = emitCode("JMP");								//���ɴ��룺��������ת����Ҫ���
		entries[entry_top - 1].val = back_loc;
	}


	if (getNextWord().word_content != ";") {
		errorWhile(6, 2);											//';'���󣬱���ͬ��
	}
	blockAnalyzer();
	emitCode("OPR", 0, 0);											//�������룺�����������ͷ�ջ�ռ�
}
void syntaxAnalyzer() {
	progAnalyzer();
}

/*****************************************���ű����*********************************************/
void addEntry(string name, string kind, int level, int val) {
	if (kind == "procedure") {
		entries[entry_top] = entry(name, kind, level, 0, val, 0, 0);
		entry_display.push_back(entry_top + 1);											//��ÿ�����̵���ڼ�¼��display��
	}
	else {
		int off = entries[entry_top - 1].offset;
		if (kind != "const") {															//ֻ���βκͱ��������offset
			off += 1;
		}
		if (kind == "parameter") {
			int start = entry_display.back() - 1;
			entries[start].para_num++;
		}
		entries[entry_top] = entry(name, kind, level, off, val, entry_top + 1, 0);
	}
	entry_top++;
	entries[entry_top].previous = 0;
}
void delEntry() {
	int len = entry_display.size();
	//���Ķ�Ӧ�ķ��ű�top
	entry_top = entry_display[len - 1];
	//����������Ӧ��previous��ֵ���б��
	entries[entry_top - 1].previous = entry_top;
	//ɾ��������ʾ���еļ�¼
	entry_display.pop_back();
}
pair<int, int> searchEntry(string name) {
	//�ڸñ��ϲ���
	int len = entry_display.size();
	int sub_level = 0;
	//���������ڷ��ű��ж�Ӧ����ʼλ��
	for (int i = len - 1; i >= 0; i--) {
		int start = entry_display[i];
		while (start != 0) {
			if (name == entries[start].name) {
				return { start,sub_level };
			}
			start = entries[start].previous;
		}
		sub_level++;
	}
	//û���ҵ������ؿ���
	return { -1,INF };
}
void updateEntry(string name, int val) {
	pair<int, int> id = searchEntry(name);
	if (id.first == -1) {
		cout << "δ�ҵ�ָ�����" << endl;
	}
	else {
		entries[id.first].val = val;
	}
}
int judgeDefined(string name) {
	//������ǰ�������Ƿ�˱����Ѿ�������
	int start = entry_display.back();
	while (start != 0) {
		if (name == entries[start].name) {
			return 1;
		}
		start = entries[start].previous;
	}

	//�����ñ�ʶ���붨��ı����Ƿ��г�ͻ
	int len = entry_display.size();
	int sub_level = 0;
	//���������ڷ��ű��ж�Ӧ����ʼλ��
	for (int i = len - 1; i >= 0; i--) {
		start = entry_display[i];
		while (start != 0) {
			if (name == entries[start].name && entries[start].kind == "const") {
				return 1;
			}
			start = entries[start].previous;
		}
		sub_level++;
	}
	return -1;
}

/**************************************�����м�������****************************************************/
int emitCode(string op_name, int level, int op_num) {
	codes.push_back(code(op_name, level, op_num));
	return codes.size() - 1;
}
void backPatch(int id, int val) {
	codes[id].op_num = val;
}
int oprType(string op) {											//ѡ��opr�Ĳ���
	if (op == "end") {
		return 0;
	}
	else if (op == "res") {
		return 1;
	}
	else if (op == "+") {
		return 2;
	}
	else if (op == "-") {
		return 3;
	}
	else if (op == "*") {
		return 4;
	}
	else if (op == "/") {
		return 5;
	}
	else if (op == "odd") {
		return 6;
	}
	else if (op == "=") {
		return 7;
	}
	else if (op == "<>") {
		return 8;
	}
	else if (op == "<") {
		return 9;
	}
	else if (op == ">=") {
		return 10;
	}
	else if (op == ">") {
		return 11;
	}
	else if (op == "<=") {
		return 12;
	}
	else if (op == "#") {
		return 13;
	}
	else if (op == "0") {
		return 14;
	}
	else {
		cout << "ϵͳ����" << endl;
		exit(0);
	}
}
int getSize() {
	//�õ���ǰ����Ҫ���ٵĿռ��С
	int start = entry_display.back();
	int size = 0;
	while (start != 0) {
		size = max(size, entries[start].offset);
		start = entries[start].previous;
	}
	return size + 3;
}

/***************************************�м���������************************************************/
int findVal(int level, int off) {
	int start = sp;
	while (level--) {
		start = act_stack[start];
	}
	return act_stack[start + off];
}
void setVal(int val, int level, int off) {
	int start = sp;
	while (level--) {
		start = act_stack[start];
	}
	act_stack[start + off] = val;
}
pair<int, int>getTwoTop() {
	int num1 = data_stack.top();
	data_stack.pop();
	int num2 = data_stack.top();
	data_stack.pop();
	return { num1,num2 };
}
int getStaticChain(int level) {
	int ans = sp;
	while (level--) {
		ans = act_stack[ans];
	}
	return ans;
}
int codeAnalyzer(int code_id) {
	int next_id = code_id + 1;
	if (codes[code_id].op_name == "LIT") {
		//������ȡ��ջ��
		data_stack.push(codes[code_id].op_num);
	}
	else if (codes[code_id].op_name == "LOD") {
		//������ֵȡ��ջ����������Ҫ���ݲ��ȥ��
		int num = findVal(codes[code_id].level, codes[code_id].op_num);
		data_stack.push(num);
	}
	else if (codes[code_id].op_name == "STO") {
		//����픃�������ĳ׃����Ԫ��
		int num = data_stack.top();
		data_stack.pop();
		setVal(num, codes[code_id].level, codes[code_id].op_num);
	}
	else if (codes[code_id].op_name == "CAL") {
		//��̬��
		int num = getStaticChain(codes[code_id].level);
		act_stack[act_top] = num;
		//��̬��
		act_stack[act_top + 1] = sp;
		sp = act_top;
		//���ص�ַ
		act_stack[act_top + 2] = next_id;
		//���ù���
		next_id = codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "INT") {
		//������ջ��Ϊ�����ó��򿪱ٿռ�
		act_top += codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "JMP") {
		//��������ת��ָ����ַ
		next_id = codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "JPC") {
		//��������ת��䣬ջ����ֵΪ�ٵ�ʱ����ת
		int num = data_stack.top();
		data_stack.pop();
		next_id = (num == 0) ? codes[code_id].op_num : next_id;
	}
	else if (codes[code_id].op_name == "WRT") {
		//���ջ��Ԫ��
		cout << "��������";
		cout << data_stack.top();
		data_stack.pop();
	}
	else if (codes[code_id].op_name == "OPR") {
		int num;
		pair<int, int>temp;
		switch (codes[code_id].op_num) {
		case 0:									//���̵��ý��������ص��õ�
			act_top = sp;						//��ջ
			next_id = act_stack[sp + 2];		//���ݷ��ص�ַ���õ���һ��ָ���λ��
			sp = act_stack[sp + 1];				//sp���ݶ�̬������
			break;
		case 1:									//ջ��Ԫ��ȡ��
			num = data_stack.top();
			data_stack.pop();
			data_stack.push(~num);
			break;
		case 2:									//��ջ����ջ����ӣ���������ջ��
			temp = getTwoTop();					//ǰһ������ջ������һ����ջ��
			data_stack.push(temp.second + temp.first);
			break;
		case 3:									//��ջ����ջ���������������ջ��
			temp = getTwoTop();					//ǰһ������ջ������һ����ջ��
			data_stack.push(temp.second - temp.first);
			break;
		case 4:									//��ջ����ջ����ˣ���������ջ��
			temp = getTwoTop();					//ǰһ������ջ������һ����ջ��
			data_stack.push(temp.second * temp.first);
			break;
		case 5:									//��ջ����ջ���������������ջ��
			temp = getTwoTop();					//ǰһ������ջ������һ����ջ��
			data_stack.push(temp.second / temp.first);
			break;
		case 6:									//�ж�ջ������ż�ԣ�����������1������Ϊ0
			num = data_stack.top();
			data_stack.push((num % 2 == 1));		//�ж�
			break;
		case 7:									//�ж�ջ�����ջ���Ƿ���ȣ���ȷ���1
			temp = getTwoTop();
			data_stack.push((temp.first == temp.second));
			break;
		case 8:									//�ж�ջ�����ջ���Ƿ���ȣ�����ȷ���1
			temp = getTwoTop();
			data_stack.push(!(temp.first == temp.second));
			break;
		case 9:									//�жϴ�ջ���Ƿ�С��ջ����
			temp = getTwoTop();
			data_stack.push((temp.second < temp.first));
			break;
		case 10:								//�жϴ�ջ���Ƿ���ڵ���ջ����
			temp = getTwoTop();
			data_stack.push((temp.second >= temp.first));
			break;
		case 11:								//�жϴ�ջ���Ƿ����ջ����
			temp = getTwoTop();
			data_stack.push((temp.second > temp.first));
			break;
		case 12:								//�жϴ�ջ���Ƿ�С�ڵ���ջ����
			temp = getTwoTop();
			data_stack.push((temp.second <= temp.first));
			break;
		case 13:									//�����Ļ����
			cout << endl;
			break;
		default:break;
		}
	}
	else if (codes[code_id].op_name == "RED") {
		//�����ж�ȡһ�����룬���������ջ��
		int num;
		cout << "�����룺";
		cin >> num;
		data_stack.push(num);
	}
	else if (codes[code_id].op_name == "ARG") {
		//��ջ����ֵ�������
		act_stack[act_top + codes[code_id].op_num] = data_stack.top();
		data_stack.pop();
	}
	return next_id;
}
void interpreter() {
	int code_start = 0;
	while (1) {
		code_start = codeAnalyzer(code_start);
		if (code_start == codes.size() - 1) {
			codeAnalyzer(code_start);
			break;
		}
	}
}

/***************************************���Դ���********************************************************/
void test() {
	initErrors();
	readProperitesToSet("�ؼ���.txt", "�����.txt");
	lexicalAnalysis("../../../��������/test_��������.txt");
	syntaxAnalyzer();
	cout << "============================================�м����========================================" << endl;
	cout << setw(10) << left << "����loc" << setw(10) << left << "ָ������" << setw(10) << left << "���" << setw(10) << left << "������" << endl;
	for (int i = 0; i < codes.size(); i++) {
		cout << setw(10) << left << i << " ";
		codes[i].printCode(10);
	}
	cout << "===============================================end===========================================" << endl;
	interpreter();
}

signed main() {
	test();
	return 0;
}