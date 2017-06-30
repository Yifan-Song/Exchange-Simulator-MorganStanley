#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <time.h>
using boost::asio::ip::tcp;

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
	while (true)
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

void Transform(std::string item)//将接受到的信息翻译并输出
{
	using namespace std;
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
		cout << "\nA requset to create a new order is received:\nOrderID:" << localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tAmount:" << localAmount << "\tPrice:" << localPrice << endl;
	}
	else if (Read(item, "35") == "8"&&Read(item, "39") == "0"&&Read(item, "150") == "0")
	{
		cout << "\nThe requset to create a new order which ID is " << localOrderID << " has been executed successfully.\n";
	}
	else if (Read(item, "35") == "F")
	{
		cout << "\nA requset to cancel a order is received:\nOrderID:" << localOrderID << ".\n";
	}
	else if (Read(item, "35") == "8"&&Read(item, "39") == "4"&&Read(item, "150") == "4")
	{
		cout << "\nThe requset to cancel the order which ID is " << localOrderID << " has been executed successfully.\n";
	}
	else if (Read(item, "35") == "9"&&Read(item, "39") == "8")
	{
		cout << "\nThe requset to cancel the order which ID is " << localOrderID << " has been rejected.\n";
	}
	else if (Read(item, "QueryName") != "-1")
	{
		cout << "\nA request to query the orderbook of " << Read(item, "QueryName") << " is received.\n";
	}
	else if (Read(item, "35") == "8"&&Read(item, "39") == "1")
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
		tempStream.clear();
		tempStream.str("");
		DealedAmount = OriginAmount - LeftAmount;
		cout << "\nAn Order has been partially filled:\nOrderID:" << localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tDealedAmount:" << DealedAmount << "\tLeftAmount:" << localAmount << "\tDealPrice:" << localPrice << endl;
	}
	else if (Read(item, "35") == "8"&&Read(item, "39") == "2")
	{
		cout << "\nAn Order has been fully filled:\nOrderID:" << localOrderID << "\tOrderType:" << localType << "\tGoodName:" << localGoodName << "\tAmount:" << localAmount << "\tDealPrice:" << localPrice << endl;
	}
	else if(item=="End"){}
	else
	{
		cout << "\nUnknown situation.\n";
	}
}

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
	tcp::resolver::query query(IPaddress, "9875");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::socket socket(io_service);
	boost::asio::connect(socket, endpoint_iterator);
	boost::system::error_code error;
	std::array<char, 256> input_buffer;
	
	std::cout << "Monitoring...\n";

	std::string message = "Monitor";
	boost::asio::write(socket, boost::asio::buffer(message), error);
	while (true)
	{
		std::size_t rsize = socket.read_some(
			boost::asio::buffer(input_buffer), error);
		std::string receivedMessage(input_buffer.data(), input_buffer.data() + rsize);//接受信息并转化后输出
		Transform(receivedMessage);
	}
	return 0;
}