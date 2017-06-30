#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <time.h>
#include <thread>
#include <algorithm>
using boost::asio::ip::tcp;

std::vector<std::string> history;//���ڴ洢cliet��server������fixЭ��ͽ��յ���fixЭ��
std::string MsgType;//���µ�string�����ڴ洢����tag�Ķ�Ӧֵ���ַ���  ��������δ�ҵ���Ӧ��tag���Լ��½���queryname��goodname��originamount��originprice4��tag
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
std::stringstream Fix;//���ڴ洢ÿһ�η�����fixЭ��
int maxAmount = 1000000;//�����ж��û�����Ľ�����Ŀ�۸��Ƿ�Ϸ�
int minAmount = 100;
int maxPrice = 1000;
int minPrice = 0;

bool isOpen()//�����жϵ���ʱ���г��Ƿ񿪷�
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

bool isNumber(std::string s)//���ڼ���û�������ַ����Ƿ�Ϊ�Ϸ�������
{
	using namespace std;
	if (s.find('.') != s.npos)
	{
		if (s.find_first_of('.') != s.find_last_of('.') || s.find('.') == 0 || s.find('.') == s.length() - 1)return false;//С����ֻ����һλ�Ҳ����ڵ�һ�����һλ
	}
	for (int i = 0;i < s.length(); i++)
	{
		if ((int(s[i])>47 && int(s[i]) < 58) || int(s[i]) == 46)continue;
		else return false;
	}
	return true;
}

int StringToNumber(std::string s)//���ڽ��Ϸ����ַ���ת��������
{
	using namespace std;
	int num;
	stringstream tempStream;
	tempStream << s;
	tempStream >> num;
	return num;
}

std::string Read(std::string s, std::string aim)//���ڷ���fixЭ����tag��Ӧ��ֵ������ֵsΪfixЭ�飬aimΪtag
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
	while (true)//��ѭ�����ڴ�������11=150��150=23��������fixЭ��Ĳ�ѯ
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

void ToLowerString(std::string &str)//���û�������ַ���׼�������ж�
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))tolower);
}

void Create()//�������¶����Ĺ���
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
		const time_t t = time(NULL);//���ڻ���ʱ�����ɶ�����
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

void Cancle()//���ڴ���ɾ�������Ĺ���
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

void ReadHistory()//���ڶ�ȡ��ʷ������֮�����
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
		std::cout << "The market is closed now.\nOpentime:9:30AM-11:30AM && 1:00PM-3:30PM\n";//��ʱ�䲻�Ϸ�ʱֱ���˳�
		return 0;
	}
	std::string IPaddress;
	std::cout << "Please enter the IP address:";//����ip��ַ������
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
	std::string receivedMessage1(input_buffer.data(), input_buffer.data() + rsize);//�˴����ڶ�ȡ�ȴ����ӿͻ������ߵ���Ϣ
	std::cout << receivedMessage1;
	rsize = socket.read_some(
		boost::asio::buffer(input_buffer), error);
	std::string receivedMessage2(input_buffer.data(), input_buffer.data() + rsize);//�˴����ڶ�ȡ׼����ɵ���Ϣ
	std::cout << receivedMessage2;
	while (true)
	{
		Fix.clear();//ÿһ��ѭ�������fixЭ��
		Fix.str("");
		while (true)
		{
			std::cout << "Query or Deal?(Enter q for query, d for deal)\n";//�ȸ��û���ѯ��Ϣ���׵�ѡ��
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
				std::cout << "Query the history or the orderbook?(Enter h for history, o for orderbook)\n";//��ѡ���ѯ��Ϣ�������ѯ������ʷ��orderbook��ѡ��
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
				ReadHistory();//���������ʷ
				std::cout << "\n";
				continue;
			}
			if (QueryStr == "o")
			{
				while (true)
				{
					std::cout << "Please Enter the name of good you want to query:\n";//���û�����Ҫ��ѯ�Ļ�����
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
				boost::asio::write(socket, boost::asio::buffer(Fix.str()), error);//����orderbook�����
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
				std::cout << "Create a new order or Cancel an old order?(Enter n for create, o for cancle)\n";//���û������¶�����ȡ���ɶ�����ѡ��
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
			Fix << Read(Fix.str(), "38");//������tag�Ľ���������partialfillʱ����ԭʼ��Ŀ��۸�ͳɽ���Ŀ��۸��
			Fix << ";";
			Fix << "OriginPrice=";
			Fix << Read(Fix.str(), "44");
			Fix << ";";
			history.push_back(Fix.str());
			boost::asio::write(socket, boost::asio::buffer(Fix.str()), error);
			std::size_t rsize = socket.read_some(
				boost::asio::buffer(input_buffer), error);
			std::string receivedMessage(input_buffer.data(), input_buffer.data() + rsize);//������Ϣ������ʵʱ���������µĴ��뼴Ϊ���ʵʱ�����ģ�
			history.push_back(receivedMessage);
			if (Read(Fix.str(), "35") == "D")
			{
				if (receivedMessage.find("39=0") != receivedMessage.npos)std::cout << "Order created successfully.\n\n";
				while (receivedMessage != "End")//���ѭ�������ڽ���server��һ���¶�������ȥ֮����ܲ����ļ���fill��Ϣ�ķ����ģ�end��־����
				{
					std::size_t rsize = socket.read_some(
						boost::asio::buffer(input_buffer), error);
					std::string receivedMessage(input_buffer.data(), input_buffer.data() + rsize);
					history.push_back(receivedMessage);
					std::string Type;
					if (Read(receivedMessage, "40") == "1")Type = "Buy";
					else Type = "Sell";
					if (Read(receivedMessage, "35") == "8"&&Read(receivedMessage, "39") == "1")//����partialfill�����
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
					else if (Read(receivedMessage, "35") == "8"&&Read(receivedMessage, "39") == "2")//����fullyfill�����
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
			else if (receivedMessage.find("35=8") != receivedMessage.npos&&receivedMessage.find("39=4") != receivedMessage.npos&&receivedMessage.find("150=4") != receivedMessage.npos)std::cout << "Order cancelled successfully.\n\n";//����ȡ�������ɹ������
			else if (receivedMessage.find("35=9") != receivedMessage.npos&&receivedMessage.find("39=8") != receivedMessage.npos)std::cout << "The order can't be cancelled.(Doesn't exist or has been cancelled.)\n\n";//����ȡ������ʧ�ܵ����
			else std::cout << "\nUnknown situation\n";
		}
	}
	return 0;
}