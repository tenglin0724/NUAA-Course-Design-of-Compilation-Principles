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
const int INF = 1e9 + 7;								//定义最大整数值
const int MAXN = 1e5 + 5;								//定义符号表表项的最大值


/**************************************相关表项、错误结果定义****************************************/
struct Word {
	string word_content;						//单词内容
	string word_type;							//单词种类
	int row;									//单词所在行数
	Word() {}
	Word(string wc, int row, string wt) {
		word_content = wc;
		this->row = row;
		word_type = wt;
	}
	void printWord() {
		//打印识别的单词结果
		cout << setw(10) << left << word_content << "     " << setw(10) << word_type << "       " << row << endl;
	}
	bool operator==(const Word& word) {
		return this->word_content == word.word_content;
	}
};
vector<Word>words;										//定义存储词法分析器的结果数组
struct Error {
	string error_content;						//定义错误内容
	string error_type;							//定义错误类型
	int row;									//定义错误所在行数
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
vector<Error>errors;									//定义存储错误的结构体

struct entry {
	string name;							//变量名
	string kind;							//该项的种类
	int level;								//该项所在层的大小
	int offset;								//该项的相对地址(自动生成)
	int val;								//若为变量或者常量，需要赋值
	int previous;							//链，可以通过此链去查找，如果值为零，表示该过程中没有指定内容（自动生成)
	int para_num;							//过程名对应的值，表示该过程对应的参数数量
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
vector<entry>entries(MAXN);								//栈式符号表	
vector<int>entry_display;								//定义对应的显示符号表

struct code {
	string op_name;					//操作名
	int level;						//层数
	int op_num;						//操作数
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
vector<code>codes;										//定义三地址代码

unordered_map<int, string>all_errors;					//定义错误的键值对集合
unordered_map<int, string>all_errors_tp;				//定义错误类型表

unordered_set <string>reserved;							//定义关键字的集合
unordered_set <string>symbol;							//定义符号的集合

vector<int>act_stack(MAXN);								//定义活动记录栈
stack<int>data_stack;									//定义数据栈

int act_top = 0;										//定义活动栈栈顶指针
int sp = 0;												//定义当前过程的起始指针
int entry_top = 0;										//定义栈式符号表的栈顶
int cru_level = 0;										//嵌套层数

int point = 0;											//定义全局指针
int row = 0;											//定义当前识别行
int word_point = 0;										//定义每一个语法单位的指针
Word tword;												//全局的单词缓存

/*******************************************定义函数声明**************************************************/
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


/*******************************************工具函数**************************************************/
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
	//定义相应的错误初始化程序，语法分析其运行前将所有错误加入表中
	all_errors[1] = "Identifier Cannot Start With A Number";			//标识符数字开头
	all_errors[2] = "Assignment Symbol Is Missing ‘=’.";				//赋值符号错误
	all_errors[3] = "Unknown Lexical Error";							//未知的词法错误

	all_errors[4] = "Invalid Or Missing RESERVED 'program'";			//program错误
	all_errors[5] = "Identifier Expected";								//缺少标识符
	all_errors[6] = "';' Symbol Expected";								//缺少';'
	all_errors[7] = "':=' Symbol Expected";								//缺少'：='
	all_errors[8] = "Invalid Or Missing INTEGER";						//缺少数字
	all_errors[9] = "',' Symbol Expected";								//缺少','
	all_errors[10] = "Unexpected Identifier or Symbol";					//未定义的符号
	all_errors[11] = "'(' Symbol Expected";								//缺少'('
	all_errors[12] = "Argument Is Missing";								//缺少参数
	all_errors[13] = "')' Symbol Expected";								//缺少')'
	all_errors[14] = "Invalid Or Missing RESERVED 'begin'";				//缺少begin
	all_errors[15] = "LOP Symbol Expected";								//缺少lop
	all_errors[16] = "Invalid Or Missing RESERVED 'then'";				//缺少then
	all_errors[17] = "Invalid Or Missing RESERVED 'do'";				//缺少do
	all_errors[18] = "Has Already Defined";								//重定义
	all_errors[19] = "Invalid Or Missing RESERVED 'end'";				//缺少end
	all_errors[20] = "Too many parameters passed in";					//传入参数过多
	all_errors[21] = "Too few parameters passed in";					//传入参数太少
	all_errors[22] = "Undefined Identifier";							//标识符未定义




	all_errors_tp[1] = "Syntax Error";									//语法错误
	all_errors_tp[2] = "Lexical Error";									//词法错误
	all_errors_tp[3] = "Semantic Error";								//语义错误
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
	exit(0);											//有错误直接终止程序
}

/********************************************词法分析**************************************************/
void trim(string& s)
{
	//去字符串两边的空格和制表符
	if (s.empty())
	{
		return;
	}
	s.erase(0, s.find_first_not_of(" \t"));
	s.erase(s.find_last_not_of(" \t") + 1);

}
bool isDigit(char ch) {
	//识别字符是否为数字
	if (ch >= '0' && ch <= '9') {
		return 1;
	}
	return 0;
}
bool isLetter(char ch) {
	//识别是否是字符
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
		return 1;
	}
	return 0;
}
bool isSymbol(char ch) {
	//识别是否为符号中的字符
	for (auto i : symbol) {
		if ((i.find(ch) != string::npos)) {
			return 1;
		}
	}
	return 0;
}
void readProperitesToSet(string url1, string url2) {
	//定义读取属性文件的方法，装载入对应集合中
	fstream file;
	file.open(url1, ios::in);
	if (file.fail()) {
		cout << url1 << " 文件打开失败！" << endl;
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
		cout << url2 << " 文件打开失败！" << endl;
		exit(0);
	}
	while (getline(file, temp)) {
		trim(temp);
		symbol.insert(temp);
	}
	file.close();
}
void checkIsNumber(string str) {
	//检查当前字段是否为数字
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
			//这里识别出错误之后，保存错误，将指针跳转到空格位置
			pushError(1, 2);
			while (point + 1 < str.size() && str[point + 1] != ' ') {
				point++;
			}
			return;
		}
	}
	//这种表示当前token已经识别结束，且识别成功
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
	//检查当前字段是否为合法标识符
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
	//检查当前字段是否为合法符号
	string token;
	char ch = str[point];
	token.push_back(str[point++]);
	//分支判断
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
		//找到空格
		ch = str[point];
		if (ch == ' ') {
			return;
		}
		else {
			token.push_back(ch);
		}
	}

	//添加词法错误
	pushError(3, 2, token);
}
void startAnalysis(string str) {
	if (str.size() == 0) {
		//传入字符串为空直接返回
		return;
	}
	else {
		char ch;
		//取出第一个字符
		for (; point < str.size(); point++) {
			ch = str[point];
			if (isDigit(ch)) {
				//开头是数字，判断该字段是否数字
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
	//读取测试文件，进行分析
	fstream file;
	file.open(url, ios::in);
	if (file.fail()) {
		cout << url << "文件打开失败！" << endl;
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


/********************************************语义和语法分析**********************************************/
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
	//error_id是报错内容的编号，syn_id是同步的判断符号
	pushError(error_id);
	while (synchronous(syn_id)) {
		tword = getNextWord();
	}
	word_point--;
}
Word getNextWord() {
	//返回下一个语法单位
	if (word_point >= words.size()) {
		//如果已经没有单词可以读取，则直接返回。
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
		errorWhile(5, 3);														//const表达式缺少id，报错并同步
	}
	else {
		if (searchEntry(tword.word_content).first != -1) {						//这里需要判断该标识符是否已被定义，如果在代码中出现过就报错，重定义
			pushError(18, 3, tword.word_content);
		}
		else {
			addEntry(tword.word_content, "const", cru_level);
		}
	}

	if (getNextWord().word_content != ":=") {
		errorWhile(7, 4);														//const表达式':='缺少或者不正确，报错并同步
	}
	if (getNextWord().word_type != "INTEGER") {
		errorWhile(9, 5);														//const表达式缺少int，报错并同步
	}
	else {
		//把变量对应的值填入
		entries[entry_top - 1].val = changeNum(tword.word_content);
	}

	if (getNextWord().word_content == ",") {
		constAnalyzer();														//识别到下一个const,递归分析
	}
	else if (tword.word_content == ";") {
		return;																	//识别到';'，直接结束
	}
	else if (tword.word_type == "IDENTIFIER") {
		pushError(9);															//缺少分隔符号',',报错并回退
		word_point--;
		constAnalyzer();
	}
	else {																		//其他符号，报错并同步
		errorWhile(10, 6);
	}
}
void condeclAnalyzer() {
	constAnalyzer();
}
void vardeclAnalyzer() {
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 5);													//缺少id，报错并同步
	}
	else {
		if (judgeDefined(tword.word_content) == -1) {						//判断变量是否重定义
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
	else if (tword.word_content == ";") {									//var分析结束
		return;
	}
	else if (tword.word_type == "IDENTIFIER") {								//缺少','
		pushError(9);
		word_point--;
		vardeclAnalyzer();
	}
	else {
		errorWhile(10, 6);
	}
}
void procedureAnalyzer() {
	cru_level++;																//嵌套层加一
	if (getNextWord().word_type != "IDENTIFIER") {								//缺少id，报错并同步
		errorWhile(5, 7);
	}
	else {
		if (judgeDefined(tword.word_content) == -1) {							//判断该过程名是否已经被定义
			addEntry(tword.word_content, "procedure", cru_level);
			//生成中间代码：无天条件跳转语句
			int back_loc = emitCode("JMP");
			//保存当前过程对应语句的位置
			entries[entry_top - 1].val = back_loc;
		}
		else {
			pushError(18, 3, tword.word_content);
		}
	}
	idPaAnalyzer("procedure");													//参数列表循环分析
	if (getNextWord().word_content != ";") {									//分析'；'
		errorWhile(6, 13);
	}
	blockAnalyzer();															//分析block
	//生成中间代码：该过程结束，释放对应的栈空间
	emitCode("OPR", 0, 0);
	delEntry();																	//分析完procedure后，删除过程对应的表项，
	cru_level--;																//分析完，当前层要减一

	tword = getNextWord();
	if (tword.word_content == ";") {											//判断下一个单词是不是procedure
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
		if (loc.first == -1) {												//判断是否被定义
			pushError(22, 3, tword.word_content);							//标识符未定义
		}
		else {
			if (entries[loc.first].kind == "const") {
				//生成中间代码：常量进数据栈
				emitCode("LIT", 0, entries[loc.first].val);
			}
			else {
				//生成中间代码：形参或者变量加载

				emitCode("LOD", loc.second, entries[loc.first].offset + 2);
			}
		}
	}
	else if (tword.word_type == "INTEGER") {
		//生成中间代码：常量进数据栈
		emitCode("LIT", 0, changeNum(tword.word_content));
	}
	else if (tword.word_content == "(") {								//(<exp>)
		expAnalyzer();
		if (getNextWord().word_content != ")") {
			errorWhile(13, 6);											//报错，缺少'）'并同步
		}
	}
	else {
		errorWhile(10, 6);												//不符合上述几种情况
	}
}
void termAnalyzer() {
	factorAnalyzer();
	tword = getNextWord();
	if (tword.word_content == "*" || tword.word_content == "/") {
		int opr = oprType(tword.word_content);
		termAnalyzer();
		//生成中间代码：算术运算
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
			//生成特殊指令，将0送入数据栈
			emitCode("OPR", 0, oprType("0"));
			flag = 1;
		}
		tword = getNextWord();										//忽略掉'+'和'-'
	}
	word_point--;
	termAnalyzer();													//分析term
	if (flag == 1) {
		//生成减法指令
		emitCode("OPR", 0, oprType("-"));
	}
	tword = getNextWord();
	if (tword.word_content == "+" || tword.word_content == "-") {	//判断接下来是否还有
		int opr = oprType(tword.word_content);
		termAnalyzer();
		//生成中间代码：加减运算
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
	else if (!synchronous(11)) {											//如果是<exp>的first
		word_point--;
		expAnalyzer();													//分析<exp>
		tword = getNextWord();
		if (!synchronous(12)) {											//判断是否为<lop>
			int opr = oprType(tword.word_content);
			expAnalyzer();												//是lop后再进行<exp>分析
			//生成中间代码：opr操作
			emitCode("OPR", 0, opr);
		}
		else {
			errorWhile(15, 13);											//否则报错，缺少<lop>
		}
	}
	else {
		errorWhile(10, 13);												//报错并同步
	}
}
void expRecursion(string select, int& off) {
	expAnalyzer();												//分析<exp>
	if (select == "call") {
		//生成中间代码：参数传递											
		emitCode("ARG", 0, off);
		off++;
	}
	else {
		//生成中间代码：wrt输出栈顶
		emitCode("WRT", 0, 0);
	}


	tword = getNextWord();										//分析后面的符号
	if (tword.word_content == ",") {
		expRecursion(select, off);
	}
	else if (tword.word_content == ")") {						//退出
		return;
	}
	else if (tword.word_content == ";") {						//报错，缺少')'并回退
		pushError(13);
		word_point--;
		return;
	}
	else if (!synchronous(11)) {								//缺少','
		pushError(9);
		word_point--;
		expRecursion(select, off);
	}
	else {
		errorWhile(10, 6);
	}
}
void expPaAnalyzer(string select, int& off) {
	if (getNextWord().word_content != "(") {							//分析'('
		errorWhile(11, 15);
	}
	tword = getNextWord();
	if (tword.word_content == ")") {
		if (select == "write") {
			pushError(12);												//缺少<exp>
		}
	}
	else if (tword.word_content == ";") {
		if (select == "write") {
			pushError(12);												//缺少<exp>
		}
		else {
			pushError(13);												//缺少')'
		}
		word_point--;
	}
	else {																//其他情况调用exp
		word_point--;
		expRecursion(select, off);
	}
}
void idRecursion(string select) {
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 9);														//缺少id，报错并同步
	}
	else {
		if (select == "procedure") {											//判断是否标识符已经被定义
			if (judgeDefined(tword.word_content) == -1) {
				addEntry(tword.word_content, "parameter", cru_level);
			}
			else {
				pushError(18, 3, tword.word_content);
			}

		}
		else {																	//如果是read，那么生成代码
			pair<int, int>loc = searchEntry(tword.word_content);
			if (loc.first == -1) {
				pushError(22, 3, tword.word_content);
			}
			else {
				//生成中间代码：red读取代码
				emitCode("RED", 0, 0);
				//生成中间代码：sto读取栈顶元素到指定位置
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
	//为了代码复用加的判断参数，select为'procedure'表示参数列表可以为空，'read'表示不能为空
	if (getNextWord().word_content != "(") {								//缺少'('，报错并同步				
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
			pushError(19);									//报错，缺少end
			word_point--;
			break;
		}
		else {
			pushError(6);									//报错，缺少';'
			word_point--;
			stateAnalyzer();
		}
	}
}
void stateAnalyzer() {
	tword = getNextWord();
	if (tword.word_content == "if") {
		lexpAnalyzer();													//分析<lexp>	
		//生成中间代码：判断栈顶是否为真（需要回填）假出口
		int back_loc_false = emitCode("JPC");
		if (getNextWord().word_content != "then") {						//分析'then'
			errorWhile(16, 10);
		}
		stateAnalyzer();
		if (getNextWord().word_content == "else") {						//分析'else'
			//生成中间代码：判断栈顶是否为真（需要回填）真出口
			int back_loc_true = emitCode("JMP");
			//回填假出口
			backPatch(back_loc_false, codes.size());
			stateAnalyzer();
			//回填真出口
			backPatch(back_loc_true, codes.size());
		}
		else {
			//回填假出口
			backPatch(back_loc_false, codes.size());
		}
	}
	else if (tword.word_content == "while") {
		//记录无条件跳转的回填地址
		int return_loc = codes.size();
		lexpAnalyzer();															//分析<lexp>	
		//生成中间代码：有条件跳转（待回填）
		int back_loc = emitCode("JPC");
		if (getNextWord().word_content != "do") {
			errorWhile(17, 10);													//分析'do',没有报错同步
		}
		stateAnalyzer();														//分析<statement>
		//while后面要加一条无条件跳转语句
		int back_loc_return = emitCode("JMP");
		backPatch(back_loc_return, return_loc);
		//回填循环出口
		backPatch(back_loc, codes.size());
	}
	else if (tword.word_content == "call") {
		pair<int, int> id_loc;
		int pa_off = 3;
		if (getNextWord().word_type != "IDENTIFIER") {							//分析<id>
			errorWhile(5, 14);
		}
		else {
			id_loc = searchEntry(tword.word_content);
			if (id_loc.first == -1) {											//判断过程是否被定义
				pushError(22, 3, tword.word_content);
			}
		}

		expPaAnalyzer("call", pa_off);											//分析<exp>参数

		if (pa_off - 3 > entries[id_loc.first].para_num) {						//判断参数是否对齐
			pushError(20, 3, entries[id_loc.first].name);
		}
		else if (pa_off - 3 < entries[id_loc.first].para_num) {
			pushError(21, 3, entries[id_loc.first].name);
		}
		else {
			//生成中间代码：函数调用
			emitCode("CAL", id_loc.second, entries[id_loc.first].val);
		}
	}
	else if (tword.word_content == "read") {
		idPaAnalyzer("read");													//分析id参数，最少一个
	}
	else if (tword.word_content == "write") {
		int pa_num = 3;
		expPaAnalyzer("write", pa_num);											//分析表达式参数，至少为一个
		//生成中间代码：换行
		emitCode("OPR", 0, oprType("#"));
	}
	else if (tword.word_type == "IDENTIFIER") {
		pair<int, int>loc = searchEntry(tword.word_content);
		if (loc.first == -1) {
			pushError(22, 3, tword.word_content);
		}

		if (getNextWord().word_content != ":=") {								//缺少':='，报错并同步
			errorWhile(7, 16);
		}
		tword = getNextWord();
		word_point--;
		if (!synchronous(15)) {
			expAnalyzer();
		}
		//生成中间代码
		emitCode("STO", loc.second, entries[loc.first].offset + 2);
	}
	else if (tword.word_content == "begin") {
		stateAnalyzer();
		stateWhile();
	}
	else {																		//其他情况就报错并同步
		pushError(10);
		while (synchronous(15)) {
			tword = getNextWord();
		}
		word_point--;
	}
}
void bodyAnalyzer() {
	tword = getNextWord();
	if (synchronous(10)) {														//报错，未知符号找到同步
		pushError(10);
		while (synchronous(10)) {
			tword = getNextWord();
		}
	}
	if (tword.word_content != "begin") {
		word_point--;
	}
	//回填：回填JMP指令
	int code_loc = entries[entry_display.back() - 1].val;
	codes[code_loc].op_num = codes.size();

	//生成中间代码：在活动记录中开辟空间
	int size = getSize();
	emitCode("INT", 0, size);

	stateAnalyzer();														//分析<statement>
	stateWhile();															//分析{;<statement>}
}
void blockAnalyzer() {
	if (getNextWord().word_content == "const") {
		condeclAnalyzer();													//转到分析"condecl"
	}
	else {
		word_point--;
	}
	if (getNextWord().word_content == "var") {
		vardeclAnalyzer();													//转到分析"vardecl"
	}
	else {
		word_point--;
	}
	if (getNextWord().word_content == "procedure") {
		procedureAnalyzer();												//转到分析"procedure"
	}
	else {
		word_point--;
	}

	if (getNextWord().word_content != "begin") {							//处理begin
		errorWhile(14, 2);													//判断是否下面语句还有<block>的first
		if (tword.word_content != "begin") {
			blockAnalyzer();
			return;
		}
	}
	bodyAnalyzer();
}
void progAnalyzer() {
	if (getNextWord().word_content != "program") {
		errorWhile(4, 1);											//program错误，报错并同步
		word_point--;												//这里同步多回退一步，因为拼错的program会被当做id，所以judge中不能有id的判断
	}
	if (getNextWord().word_type != "IDENTIFIER") {
		errorWhile(5, 1);											//id错误，报错并同步
	}
	else {
		addEntry(tword.word_content, "procedure", cru_level);		//识别成功，进符号表
		int back_loc = emitCode("JMP");								//生成代码：无条件跳转（需要回填）
		entries[entry_top - 1].val = back_loc;
	}


	if (getNextWord().word_content != ";") {
		errorWhile(6, 2);											//';'错误，报错并同步
	}
	blockAnalyzer();
	emitCode("OPR", 0, 0);											//产生代码：结束分析，释放栈空间
}
void syntaxAnalyzer() {
	progAnalyzer();
}

/*****************************************符号表操作*********************************************/
void addEntry(string name, string kind, int level, int val) {
	if (kind == "procedure") {
		entries[entry_top] = entry(name, kind, level, 0, val, 0, 0);
		entry_display.push_back(entry_top + 1);											//将每个过程的入口记录在display中
	}
	else {
		int off = entries[entry_top - 1].offset;
		if (kind != "const") {															//只有形参和变量会计算offset
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
	//更改对应的符号表top
	entry_top = entry_display[len - 1];
	//将过程名对应的previous的值进行变更
	entries[entry_top - 1].previous = entry_top;
	//删除符号显示表中的记录
	entry_display.pop_back();
}
pair<int, int> searchEntry(string name) {
	//在该表上查找
	int len = entry_display.size();
	int sub_level = 0;
	//遍历过程在符号表中对应的起始位置
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
	//没有找到，返回空项
	return { -1,INF };
}
void updateEntry(string name, int val) {
	pair<int, int> id = searchEntry(name);
	if (id.first == -1) {
		cout << "未找到指定表项！" << endl;
	}
	else {
		entries[id.first].val = val;
	}
}
int judgeDefined(string name) {
	//搜索当前过程中是否此变量已经被定义
	int start = entry_display.back();
	while (start != 0) {
		if (name == entries[start].name) {
			return 1;
		}
		start = entries[start].previous;
	}

	//搜索该标识符与定义的变量是否有冲突
	int len = entry_display.size();
	int sub_level = 0;
	//遍历过程在符号表中对应的起始位置
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

/**************************************生成中间代码操作****************************************************/
int emitCode(string op_name, int level, int op_num) {
	codes.push_back(code(op_name, level, op_num));
	return codes.size() - 1;
}
void backPatch(int id, int val) {
	codes[id].op_num = val;
}
int oprType(string op) {											//选择opr的操作
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
		cout << "系统错误！" << endl;
		exit(0);
	}
}
int getSize() {
	//得到当前过程要开辟的空间大小
	int start = entry_display.back();
	int size = 0;
	while (start != 0) {
		size = max(size, entries[start].offset);
		start = entries[start].previous;
	}
	return size + 3;
}

/***************************************中间代码解释器************************************************/
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
		//将常数取到栈顶
		data_stack.push(codes[code_id].op_num);
	}
	else if (codes[code_id].op_name == "LOD") {
		//将变量值取到栈顶，这里需要根据层差去找
		int num = findVal(codes[code_id].level, codes[code_id].op_num);
		data_stack.push(num);
	}
	else if (codes[code_id].op_name == "STO") {
		//將棧頂內容送入某變量單元中
		int num = data_stack.top();
		data_stack.pop();
		setVal(num, codes[code_id].level, codes[code_id].op_num);
	}
	else if (codes[code_id].op_name == "CAL") {
		//静态链
		int num = getStaticChain(codes[code_id].level);
		act_stack[act_top] = num;
		//动态链
		act_stack[act_top + 1] = sp;
		sp = act_top;
		//返回地址
		act_stack[act_top + 2] = next_id;
		//调用过程
		next_id = codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "INT") {
		//在运行栈中为被调用程序开辟空间
		act_top += codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "JMP") {
		//无条件跳转到指定地址
		next_id = codes[code_id].op_num;
	}
	else if (codes[code_id].op_name == "JPC") {
		//有条件跳转语句，栈顶的值为假的时候跳转
		int num = data_stack.top();
		data_stack.pop();
		next_id = (num == 0) ? codes[code_id].op_num : next_id;
	}
	else if (codes[code_id].op_name == "WRT") {
		//输出栈顶元素
		cout << "输出结果：";
		cout << data_stack.top();
		data_stack.pop();
	}
	else if (codes[code_id].op_name == "OPR") {
		int num;
		pair<int, int>temp;
		switch (codes[code_id].op_num) {
		case 0:									//过程调用结束，返回调用点
			act_top = sp;						//退栈
			next_id = act_stack[sp + 2];		//根据返回地址，得到下一条指令的位置
			sp = act_stack[sp + 1];				//sp根据动态链返回
			break;
		case 1:									//栈顶元素取反
			num = data_stack.top();
			data_stack.pop();
			data_stack.push(~num);
			break;
		case 2:									//次栈顶与栈顶相加，结果存放在栈中
			temp = getTwoTop();					//前一个是首栈顶，后一个是栈顶
			data_stack.push(temp.second + temp.first);
			break;
		case 3:									//次栈顶与栈顶相减，结果存放在栈中
			temp = getTwoTop();					//前一个是首栈顶，后一个是栈顶
			data_stack.push(temp.second - temp.first);
			break;
		case 4:									//次栈顶与栈顶相乘，结果存放在栈中
			temp = getTwoTop();					//前一个是首栈顶，后一个是栈顶
			data_stack.push(temp.second * temp.first);
			break;
		case 5:									//次栈顶与栈顶相除，结果存放在栈中
			temp = getTwoTop();					//前一个是首栈顶，后一个是栈顶
			data_stack.push(temp.second / temp.first);
			break;
		case 6:									//判断栈顶的奇偶性，是奇数返回1，否则为0
			num = data_stack.top();
			data_stack.push((num % 2 == 1));		//判断
			break;
		case 7:									//判断栈顶与次栈顶是否相等，相等返回1
			temp = getTwoTop();
			data_stack.push((temp.first == temp.second));
			break;
		case 8:									//判断栈顶与次栈顶是否不相等，不相等返回1
			temp = getTwoTop();
			data_stack.push(!(temp.first == temp.second));
			break;
		case 9:									//判断次栈顶是否小于栈顶，
			temp = getTwoTop();
			data_stack.push((temp.second < temp.first));
			break;
		case 10:								//判断次栈顶是否大于等于栈顶，
			temp = getTwoTop();
			data_stack.push((temp.second >= temp.first));
			break;
		case 11:								//判断次栈顶是否大于栈顶，
			temp = getTwoTop();
			data_stack.push((temp.second > temp.first));
			break;
		case 12:								//判断次栈顶是否小于等于栈顶，
			temp = getTwoTop();
			data_stack.push((temp.second <= temp.first));
			break;
		case 13:									//输出屏幕换行
			cout << endl;
			break;
		default:break;
		}
	}
	else if (codes[code_id].op_name == "RED") {
		//命令行读取一个输入，并将其放入栈顶
		int num;
		cout << "请输入：";
		cin >> num;
		data_stack.push(num);
	}
	else if (codes[code_id].op_name == "ARG") {
		//将栈顶的值传入参数
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

/***************************************测试代码********************************************************/
void test() {
	initErrors();
	readProperitesToSet("关键字.txt", "运算符.txt");
	lexicalAnalysis("../../../测试样例/test_基本样例.txt");
	syntaxAnalyzer();
	cout << "============================================中间代码========================================" << endl;
	cout << setw(10) << left << "代码loc" << setw(10) << left << "指令名称" << setw(10) << left << "层差" << setw(10) << left << "操作数" << endl;
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