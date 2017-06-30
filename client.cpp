#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <time.h>
#include <thread>
#include <algorithm>
using boost::asio::ip::tcp;

std::vector<std::string> history;//用于存储cliet向server发出的fix协议和接收到的fix协议
std::string MsgType;//此下的string是用于存储各个tag的对应值的字符串  另外由于未找到对应的tag我自己新建了queryname、goodname、originamount和originprice4个tag
std::string OrderQty;
std::string Side;
std::string Price;
std::string ClOrdID;
std::string GoodNameTag;
std::string GoodNameTagStr;
std::string MsgTypeStr;
std::string SideStr;
std::string OrderQtyStr;
std::string PriceStr;
std::string ClOrdIDStr;
std::string QueryOrDeal;
std::string QueryStr;
std::string QueryNameStr;
std::string QueryName;
std::stringstream buffer;
std::stringstream Fix;//用于存储每一次发出的fix协议
int maxAmount = 1000000;//用于判断用户输入的交易数目价格是否合法
int minAmount = 100;
int maxPrice = 1000;
int minPrice = 0;

bool isOpen()//用于判断当下时间市场是否开放
{
	using namespace std;
	time_t Edit;
	time(&Edit);
	tm gm;
	gmtime_s(&gm, &Edit);
	int hour = gm.tm_hour + 8;
	int min = gm.tm_min;
	if (hour >= 13 && hour < 15)return true;
	else if (hour == 15 && min <= 30)return true;
	else if (hour == 9 && min >= 30)return true;
	else if (hour == 10)return true;
	else if (hour == 11 && min <= 30)return true;
	else return false;
}

bool isNumber(std::string s)//用于检测用户输入的字符串是否为合法的数字
{
	using namespace std;
	if (s.find('.') != s.npos)
	{
		if (s.find_first_of('.') != s.find_last_of('.') || s.find('.') == 0 || s.find('.') == s.length() - 1)return false;//小数点只能有一位且不能在第一或最后一位
	}
	for (int i = 0;i < s.length(); i++)
	{
		if ((int(s[i])>47 && int(s[i]) < 58) || int(s[i]) == 46)continue;
		else return false;
	}
	return true;
}

int StringToNumber(std::string s)//用于将合法的字符串转化成数字
{
	using namespace std;
	int num;
	stringstream tempStream;
	tempStream << s;
	tempStream >> num;
	return num;
}

std::string Read(std::string s, std::string aim)//用于返回fix协议中tag对应的值，参数值s为fix协议，aim为tag
{
	using namespace std;
	if (s.find(aim + "=") == s.npos)
	{
		return "-1";
	}
	int position = 0;
	int temp;
	string tempStr;
	string output;
	string temps = s;
	while (true)//此循环用于处理类似11=150；150=23；这样的fix协议的查询
	{
		int tempPosition;
		tempPosition = temps.find(aim);
		position += tempPosition;
		if (s[position + aim.length()] == '=')break;
		else
		{
			position++;
			temps = s.substr(position , s.length() - position);
		}
	}
	temp = s.find(';');
	while (true)
	{
		if (temp > position)break;
		else tempStr = s.substr(temp + 1, s.length() - temp);
		temp += tempStr.find(';');
		temp++;
	}
	output = s.substr(position, temp - position);
	output = output.substr(output.find('=') + 1, output.length() - output.find('='));
	return output;
}

void ToLowerString(std::string &str)//将用户输入的字符标准化便于判断
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))tolower);
}

void Create()//处理建立新订单的过程
{
	using namespace std;
	while (MsgTypeStr == "n")
	{
		MsgType = "35=D;";
		cout << "Buy or Sell?(Enter b for buy, s for sell)\n";
		getline(cin, SideStr);
		ToLowerString(SideStr);
		if (SideStr != "b" && SideStr != "s")
		{
			cout << "Illegal input, try again.\n";
			continue;
		}
		if (SideStr == "b")
		{
			Side = "40=1;";
		}
		if (SideStr == "s")
		{
			Side = "40=2;";
		}
		while (1)
		{
			cout << "Please Enter the name of good you want to buy/sell:\n";
			getline(cin, GoodNameTagStr);
			if (GoodNameTagStr == "")
			{
				cout << "Illegal input, try again.\n";
				continue;
			}
			GoodNameTag = "GoodName=";
			GoodNameTag += GoodNameTagStr;
			GoodNameTag += ";";
			break;
		}
		while (1)
		{
			cout << "Please Enter the number of good you want to buy/sell:\n";
			getline(cin,OrderQtyStr);
			buffer.clear();
			buffer.str("");
			buffer << OrderQtyStr;
			float qty;
			buffer >> qty;
			if (qty <= 0 || int(qty) != qty || !isNumber(OrderQtyStr))
			{
				cout << "Illegal input, try again.\n";
				continue;
			}
			if (qty > maxAmount|| qty < minAmount)
			{
				cout << "The max amount is 1000000 and the min amount is 100.Try again.\n";
				continue;
			}
			OrderQty = "38=";
			OrderQty += OrderQtyStr;
			OrderQty += ";";
			break;
		}
		while (1)
		{
			cout << "Please Enter the price you want to buy/sell:\n";
			getline(cin, PriceStr);
			buffer.clear();
			buffer.str("");
			buffer << PriceStr;
			float pri;
			buffer >> pri;
			if (pri <= 0|| !isNumber(PriceStr))
			{
				cout << "Illegal input, try again.\n";
				continue;
			}
			if (pri > maxPrice)
			{
				cout << "The max price is 1000.Try again.\n";
				continue;
			}
			Price = "44=";
			Price += PriceStr;
			Price += ";";
			break;
		}
		const time_t t = time(NULL);//用于基于时间生成订单号
		buffer.clear();
		buffer.str("");
		buffer << t;
		ClOrdID = "11=";
		ClOrdID += buffer.str();
		ClOrdID += ";";
		Fix << MsgType;
		Fix << Side;
		Fix << GoodNameTag;
		Fix << OrderQty;
		Fix << Price;
		Fix << ClOrdID;
		break;
	}
}

void Cancle()//用于处理删除订单的过程
{
	using namespace std;
	while (MsgTypeStr == "o")
	{
		MsgType = "35=F;";
		ClOrdID = "11=";
		std::cout << "Please Enter the OrderID:";
		getline(cin, ClOrdIDStr);
		ClOrdID += ClOrdIDStr;
		ClOrdID += ";";
		Fix << MsgType;
		Fix << ClOrdID;
		break;
	}
}

void ReadHistory()//用于读取历史并翻译之后输出
{
	using namespace std;
	for (auto item : history)
	{
		string localType;
		string localOrderID;
		string localGoodName;
		string localAmount;
		string localPrice;
		if (Read(item, "40") == "1")localType = "Buy";
		else localType = "Sell";
		localOrderID = Read(item, "11");
		localGoodName = Read(item, "GoodName");
		localAmount = Read(item, "38");
		localPrice = Read(item, "44");
		if (Read(item, "35") == "D")
		{
			cout << "\nYou send a request to create a new order:\nOrderID:" << localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tAmount:" << localAmount << "\tPrice:" << localPrice<<endl;
		}
		else if(Read(item, "35") == "8"&&Read(item, "39") == "0"&&Read(item, "150") == "0")
		{
			cout << "\nYour order which ID is " << localOrderID << " has been created successfully.\n";
		}
		else if (Read(item, "35") == "F")
		{
			cout << "\nYou send a request to cancel the order which ID is " << localOrderID << ".\n";
		}
		else if (Read(item, "35") == "8"&&Read(item, "39") == "4"&&Read(item, "150") == "4")
		{
			cout << "\nThe order which ID is " << localOrderID << " has been cancelled successfully.\n";
		}
		else if (Read(item, "35") == "9"&&Read(item, "39") == "8")
		{
			cout << "\nYour request to cancel the order which ID is " << localOrderID << " has been rejected\n";
		}
		else if(Read(item, "35") == "8"&&Read(item, "39") == "1")
		{
			stringstream tempStream;
			int OriginAmount;
			int DealedAmount;
			int LeftAmount;
			tempStream << Read(item, "OriginAmount");
			tempStream >> OriginAmount;
			tempStream.clear();
			tempStream.str("");
			tempStream << Read(item, "38");
			tempStream >> LeftAmount;
			DealedAmount = OriginAmount - LeftAmount;
			tempStream.clear();
			tempStream.str("");
			cout << "\nOne of your Orders has been partially filled:\nOrderID:"<<localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tDealedAmount:" << DealedAmount << "\tLeftAmount:" << localAmount << "\tDealPrice:" << localPrice << endl;
		}
		else if (Read(item, "35") == "8"&&Read(item, "39") == "2")
		{
			cout << "\nOne of your Orders has been fully filled:\nOrderID:" << localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tAmount:" << localAmount << "\tDealPrice:" << localPrice << endl;
		}
		else if (item=="End"){}
		else
		{
			cout << "\nUnknown situation.\n";
		}
	}
	return;
}

int main(int argc, char* argv[])
{
	if (!isOpen())
	{
		std::cout << "The market is closed now.\nOpentime:9:30AM-11:30AM && 1:00PM-3:30PM\n";//当时间不合法时直接退出
		return 0;
	}
	std::string IPaddress;
	std::cout << "Please enter the IP address:";//输入ip地址并连接
	std::getline(std::cin, IPaddress);
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(IPaddress, "9876");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::socket socket(io_service);
	boost::asio::connect(socket, endpoint_iterator);
	boost::system::error_code error;
	std::array<char, 256> input_buffer;
	std::size_t rsize = socket.read_some(
		boost::asio::buffer(input_buffer), error);
	std::string receivedMessage1(input_buffer.data(), input_buffer.data() + rsize);//此处用于读取等待监视客户端上线的信息
	std::cout << receivedMessage1;
	rsize = socket.read_some(
		boost::asio::buffer(input_buffer), error);
	std::string receivedMessage2(input_buffer.data(), input_buffer.data() + rsize);//此处用于读取准备完成的信息
	std::cout << receivedMessage2;
	while (true)
	{
		Fix.clear();//每一次循环后清空fix协议
		Fix.str("");
		while (true)
		{
			std::cout << "Query or Deal?(Enter q for query, d for deal)\n";//先给用户查询信息或交易的选择
			std::getline(std::cin, QueryOrDeal);
			ToLowerString(QueryOrDeal);
			if (QueryOrDeal != "q" && QueryOrDeal != "d")
			{
				std::cout << "Illegal input, try again.\n";
				continue;
			}
			break;
		}
		if (QueryOrDeal == "q")
		{
			while (true)
			{
				std::cout << "Query the history or the orderbook?(Enter h for history, o for orderbook)\n";//若选择查询信息，给其查询交易历史或orderbook的选择
				std::getline(std::cin, QueryStr);
				ToLowerString(QueryStr);
				if (QueryStr != "h" && QueryStr != "o")
				{
					std::cout << "Illegal input, try again.\n";
					continue;
				}
				break;
			}
			if (QueryStr == "h")
			{
				ReadHistory();//输出交易历史
				std::cout << "\n";
				continue;
			}
			if (QueryStr == "o")
			{
				while (true)
				{
					std::cout << "Please Enter the name of good you want to query:\n";//让用户输入要查询的货物名
					std::getline(std::cin, QueryNameStr);
					if (QueryNameStr == "")
					{
						std::cout << "Illegal input, try again.\n";
						continue;
					}
					QueryName = "QueryName=";
					QueryName += QueryNameStr;
					QueryName += ";";
					break;
				}
				Fix << QueryName;
				boost::asio::write(socket, boost::asio::buffer(Fix.str()), error);//接受orderbook并输出
				std::size_t rsize = socket.read_some(
					boost::asio::buffer(input_buffer), error);
				std::string s(input_buffer.data(), input_buffer.data() + rsize);
				std::cout << s;
			}
		}
		if (QueryOrDeal == "d")
		{
			while (true)
			{
				std::cout << "Create a new order or Cancel an old order?(Enter n for create, o for cancle)\n";//给用户建立新订单或取消旧订单的选择
				std::getline(std::cin, MsgTypeStr);
				ToLowerString(MsgTypeStr);
				if (MsgTypeStr != "n" && MsgTypeStr != "o")
				{
					std::cout << "Illegal input, try again.\n";
					continue;
				}
				break;
			}
			Create();
			Cancle();
			Fix << "OriginAmount=";
			Fix << Read(Fix.str(), "38");//这两个tag的建立是用于partialfill时区别原始数目与价格和成交数目与价格的
			Fix << ";";
			Fix << "OriginPrice=";
			Fix << Read(Fix.str(), "44");
			Fix << ";";
			history.push_back(Fix.str());
			boost::asio::write(socket, boost::asio::buffer(Fix.str()), error);
			std::size_t rsize = socket.read_some(
				boost::asio::buffer(input_buffer), error);
			std::string receivedMessage(input_buffer.data(), input_buffer.data() + rsize);//接受信息并进行实时反馈（以下的代码即为输出实时反馈的）
			history.push_back(receivedMessage);
			if (Read(Fix.str(), "35") == "D")
			{
				if (receivedMessage.find("39=0") != receivedMessage.npos)std::cout << "Order created successfully.\n\n";
				while (receivedMessage != "End")//这个循环是用于接受server在一个新订单发过去之后可能产生的几个fill信息的反馈的，end标志结束
				{
					std::size_t rsize = socket.read_some(
						boost::asio::buffer(input_buffer), error);
					std::string receivedMessage(input_buffer.data(), input_buffer.data() + rsize);
					history.push_back(receivedMessage);
					std::string Type;
					if (Read(receivedMessage, "40") == "1")Type = "Buy";
					else Type = "Sell";
					if (Read(receivedMessage, "35") == "8"&&Read(receivedMessage, "39") == "1")//处理partialfill的情况
					{
						std::stringstream tempStream;
						int OriginAmount;
						int DealedAmount;
						int LeftAmount;
						OriginAmount = StringToNumber(Read(receivedMessage, "OriginAmount"));
						LeftAmount = StringToNumber(Read(receivedMessage, "38"));
						DealedAmount = OriginAmount - LeftAmount;
						std::cout << "\nOne of your Orders has been partially filled:\nOrderID:" + Read(receivedMessage, "11") + "\tOrderType:" + Type + "\tGoodName:" + Read(receivedMessage, "GoodName") + "\tDealedAmount:" << DealedAmount << "\tLeftAmount:" + Read(receivedMessage, "38") + "\tDealPrice:" + Read(receivedMessage, "44") + "\n";
					}
					else if (Read(receivedMessage, "35") == "8"&&Read(receivedMessage, "39") == "2")//处理fullyfill的情况
					{
						std::cout << "\nOne of your Orders has been fully filled:\nOrderID:" + Read(receivedMessage, "11") + "\tOrderType:" + Type + "\tGoodName:" + Read(receivedMessage, "GoodName") + "\tAmount:" + Read(receivedMessage, "OriginAmount") + "\tDealPrice:" + Read(receivedMessage, "44") + "\n";
					}
					else if(receivedMessage=="End")
					{
						break;
					}
					else
					{
						std::cout << "\nUnknown situation.\n";
					}
					std::cout << std::endl;
				}
			}
			else if (receivedMessage.find("35=8") != receivedMessage.npos&&receivedMessage.find("39=4") != receivedMessage.npos&&receivedMessage.find("150=4") != receivedMessage.npos)std::cout << "Order cancelled successfully.\n\n";//处理取消订单成功的情况
			else if (receivedMessage.find("35=9") != receivedMessage.npos&&receivedMessage.find("39=8") != receivedMessage.npos)std::cout << "The order can't be cancelled.(Doesn't exist or has been cancelled.)\n\n";//处理取消订单失败的情况
			else std::cout << "\nUnknown situation\n";
		}
	}
	return 0;
}