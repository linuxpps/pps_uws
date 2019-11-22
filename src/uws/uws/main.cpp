/* This is a simple HTTP(S) web server much like Python's SimpleHTTPServer */

#include <App.h>

/* Helpers for this example */
#include "helpers/AsyncFileReader.h"
#include "helpers/AsyncFileStreamer.h"
#include "helpers/Middleware.h"

/* optparse */
#define OPTPARSE_IMPLEMENTATION
#include "helpers/optparse.h"

#include "common.h"
#include "curl_helper.h"
#include "signal_handler.h"

#include <map>

bool is_trade_day(const time_t& tt)
{
	struct tm* ptm = localtime(&tt);
	//printf("weekday=%d\n", ptm->tm_wday);
	if ((ptm->tm_wday >= 1) && (ptm->tm_wday <= 5))
	{
		return true;
	}
	return false;
}
time_t get_last_trade_day(const time_t& tt)
{
	time_t last_tt = tt - 86400;
	struct tm* ptm = localtime(&tt);
	//printf("weekday=%d\n", ptm->tm_wday);
	while (!is_trade_day(last_tt))
	{
		last_tt -= 86400;
	}
	return last_tt;
}
std::string time_t_to_date(const time_t& tt)
{
	struct tm* ptm = localtime(&tt);
	std::string date(16, '\0');
	snprintf(date.data(), date.size(), "%04d%02d%02d\0", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	return date.c_str();
}
void calc_date_list(std::map<std::string, std::string>& umap, const std::string & date, int count)
{
	struct tm tm = { 0 };
	tm.tm_year = std::stoi(date.substr(0, 4)) - 1900;
	tm.tm_mon = std::stoi(date.substr(4, 2)) - 1;
	tm.tm_mday = std::stoi(date.substr(6, 2));
	time_t date_tt = mktime(&tm);
	for (size_t i = 0; i < count; i++)
	{
		if (!is_trade_day(date_tt))
		{
			date_tt = get_last_trade_day(date_tt);
		}
		umap.insert(std::map<std::string, std::string>::value_type(time_t_to_date(date_tt),""));
		date_tt = get_last_trade_day(date_tt);
	}
	
}
std::string dce_chart(const std::string& product_name, const std::string& date, int count)
{
	std::string T = "";
	std::string X = "";
	std::string L5 = "";
	std::string S5 = "";
	std::string L10 = "";
	std::string S10 = "";
	std::string L20 = "";
	std::string S20 = "";
	std::string L5S = "";
	std::string L10S = "";
	std::string L20S = "";
	std::map<std::string, std::string> umap;
	setlocale(LC_ALL, "chs");
	calc_date_list(umap, date, count);
	X.append("[");
	L5.append("[");
	S5.append("[");
	L10.append("[");
	S10.append("[");
	L20.append("[");
	S20.append("[");
	L5S.append("[");
	L10S.append("[");
	L20S.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./dce.csv");
		file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/dce.csv");
		//file_reader(data, "./" + it.first + "/dce.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "合约代码：(.*?),Date：(.*?),\r\n";
			std::string pattern2 = "总计,(.*?),(.*?),\r\n";
			std::string pattern3 = "(.*?),(.*?),(.*?),(.*?),\r\n";
			std::vector<std::vector<std::string>> svv1;
			std::vector<std::vector<std::string>> svv2;
			std::vector<std::vector<std::string>> svv3;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			//printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			//printf("flag = %d\n", flag);
			int nIndex1 = (-1);
			if (svv1.size())
			{
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					if (svv1.at(0).at(i).compare(product_name) == 0)
					{
						nIndex1 = i;
						break;
					}
				}
				if (nIndex1 >= 0 && nIndex1 < svv1.at(0).size())
				{
					// 2-多单手数
					// 3-变化手数
					int nChangeIndex = 1;
					T = svv1.at(0).at(nIndex1).c_str();
					//printf("%s,%s\n", svv1.at(0).at(nIndex).c_str(), svv1.at(1).at(nIndex).c_str());
					X.append("'").append(it.first).append("',");

					flag = string_regex_find(result, svv3, out.c_str(), pattern3);
					//printf("flag = %d\n", flag);
					if (svv3.size())
					{
						int nIndexLong = nIndex1 * 63 + 21;
						int nIndexShort = nIndex1 * 63 + 42;
						int nSumLong = 0;
						int nSumShort = 0;
						//printf("%s,%s\n", svv3.at(0).at(nIndexLong).c_str(), svv3.at(1).at(nIndexLong).c_str());
						for (size_t i = 1; i <= 5; i++)
						{
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 1; i <= 5; i++)
						{
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						S5.append("'").append(std::to_string(nSumShort)).append("',");
						//持卖量
						L5.append("'").append(std::to_string(nSumLong)).append("',");
						//持买量-持卖量
						L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						for (size_t i = 6; i <= 10; i++)
						{
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 6; i <= 10; i++)
						{
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						L10.append("'").append(std::to_string(nSumLong)).append("',");
						//持卖量
						S10.append("'").append(std::to_string(nSumShort)).append("',");
						//持买量-持卖量
						L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						for (size_t i = 11; i <= 20; i++)
						{
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 11; i <= 20; i++)
						{
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						L20.append("'").append(std::to_string(nSumLong)).append("',");
						//持卖量
						S20.append("'").append(std::to_string(nSumShort)).append("',");
						//持买量-持卖量
						L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
					}

					/*flag = string_regex_find(result, svv2, out.c_str(), pattern2);
					//printf("flag = %d\n", flag);
					if (svv2.size())
					{
						int nIndex2 = nIndex1 * 3;
						//printf("%s,%s\n", svv2.at(0).at(nIndex2).c_str(), svv2.at(1).at(nIndex2).c_str());
						//成交量
						it.second = svv2.at(0).at(nIndex2);
						//持买量
						it.second = svv2.at(0).at(nIndex2 + 1);
						L20.append("'").append(svv2.at(0).at(nIndex2 + 1)).append("',");
						//持卖量
						it.second = svv2.at(0).at(nIndex2 + 2);
						S20.append("'").append(svv2.at(0).at(nIndex2 + 2)).append("',");
					}*/
				}
			}
		}
	}

	if (X.length() > 1)
	{
		*X.rbegin() = ']';
	}
	else
	{
		X.append("]");
	}
	if (L5.length() > 1)
	{
		*L5.rbegin() = ']';
	}
	else
	{
		L5.append("]");
	}
	if (S5.length() > 1)
	{
		*S5.rbegin() = ']';
	}
	else
	{
		S5.append("]");
	}
	if (L10.length() > 1)
	{
		*L10.rbegin() = ']';
	}
	else
	{
		L10.append("]");
	}
	if (S10.length() > 1)
	{
		*S10.rbegin() = ']';
	}
	else
	{
		S10.append("]");
	}
	if (L20.length() > 1)
	{
		*L20.rbegin() = ']';
	}
	else
	{
		L20.append("]");
	}
	if (S20.length() > 1)
	{
		*S20.rbegin() = ']';
	}
	else
	{
		S20.append("]");
	}

	if (L5S.length() > 1)
	{
		*L5S.rbegin() = ']';
	}
	else
	{
		L5S.append("]");
	}
	if (L10S.length() > 1)
	{
		*L10S.rbegin() = ']';
	}
	else
	{
		L10S.append("]");
	}
	if (L20S.length() > 1)
	{
		*L20S.rbegin() = ']';
	}
	else
	{
		L20S.append("]");
	}
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, X, "XXXXXX");
	string_replace_all(temp, L5, "LLL5LLL");
	string_replace_all(temp, S5, "SSS5SSS");
	string_replace_all(temp, L10, "LLL10LLL");
	string_replace_all(temp, S10, "SSS10SSS");
	string_replace_all(temp, L20, "LLL20LLL");
	string_replace_all(temp, S20, "SSS20SSS");
	string_replace_all(temp, L5S, "LLL5SSS");
	string_replace_all(temp, L10S, "LLL10SSS");
	string_replace_all(temp, L20S, "LLL20SSS");
	//file_writer(temp, T + ".html");
	return temp;
}
int main(int argc, char** argv) 
{
	//test_chart();
	//return 0;
	int option;
	struct optparse options;
	optparse_init(&options, argv);

	struct optparse_long longopts[] = {
		{"port", 'p', OPTPARSE_REQUIRED},
		{"help", 'h', OPTPARSE_NONE},
		{"passphrase", 'a', OPTPARSE_REQUIRED},
		{"key", 'k', OPTPARSE_REQUIRED},
		{"cert", 'c', OPTPARSE_REQUIRED},
		{"dh_params", 'd', OPTPARSE_REQUIRED},
		{0}
	};

	int port = 3000;
	struct us_socket_context_options_t ssl_options = {};

	while ((option = optparse_long(&options, longopts, nullptr)) != -1) {
		switch (option) {
		case 'p':
			port = atoi(options.optarg);
			break;
		case 'a':
			ssl_options.passphrase = options.optarg;
			break;
		case 'c':
			ssl_options.cert_file_name = options.optarg;
			break;
		case 'k':
			ssl_options.key_file_name = options.optarg;
			break;
		case 'd':
			ssl_options.dh_params_file_name = options.optarg;
			break;
		case 'h':
		case '?':
		fail:
			std::cout << "Usage: " << argv[0] << " [--help] [--port <port>] [--key <ssl key>] [--cert <ssl cert>] [--passphrase <ssl key passphrase>] [--dh_params <ssl dh params file>] <public root>" << std::endl;
			return 0;
		}
	}

	char* root = optparse_arg(&options);
	if (!root) {
		goto fail;
	}

	AsyncFileStreamer asyncFileStreamer(root);

	/* Either serve over HTTP or HTTPS */
	struct us_socket_context_options_t empty_ssl_options = {};
	if (memcmp(&ssl_options, &empty_ssl_options, sizeof(empty_ssl_options))) {
		/* HTTPS */
		uWS::SSLApp(ssl_options).get("/*", [&asyncFileStreamer](auto* res, auto* req) {
			serveFile(res, req);
			asyncFileStreamer.streamFile(res, req->getUrl());
			}).listen(port, [port, root](auto* token) {
				if (token) {
					std::cout << "Serving " << root << " over HTTPS a " << port << std::endl;
				}
				}).run();
	}
	else {
		/* HTTP */
		uWS::App()
			.get("/hello", [](auto* res, auto* req) {
			/* You can efficiently stream huge files too */
			res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
				})
			.get("/chart", [](auto* res, auto* req) {
					std::string_view url = req->getUrl();
					//printf("==%.*s\n", url.length(), url.data());
					std::string_view query = req->getQuery();
					//printf("==%.*s\n", query.length(), query.data());
					std::string product_name = "";
					std::string date = "";
					std::string days = "";
					std::string result = "";
					std::vector<std::vector<std::string>> svv;
					string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
					if (svv.size() > 0)
					{
						product_name = svv.at(0).at(0);
						date = svv.at(1).at(0);
						days = svv.at(2).at(0);
						try
						{
							if (std::stoi(days) > 31)
							{
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!days cannot more than 31");
							}
							else
							{
								//printf("{==%.*s==%.*s==%.*s}\n", product_name.length(), product_name.data(), date.length(), date.data(), days.length(), days.data());
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
							}
						}
						catch (const std::exception&)
						{
							res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
						}
					}
					else
					{
						res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
					}
				})
			.post("/post-chart", [](auto* res, auto* req) {
				std::string_view url = req->getUrl();
				printf("==%.*s\n", url.length(), url.data());
				std::string_view query = req->getQuery();
				printf("==%.*s\n", query.length(), query.data());
				std::string_view param0 = req->getParameter(0);
				printf("==%.*s\n", param0.length(), param0.data());
				std::string_view param1 = req->getParameter(1);
				printf("==%.*s\n", param1.length(), param1.data());

				res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart("a2005", "20191119", 4).c_str());
				})
			.get("/*", [&asyncFileStreamer](auto* res, auto* req) {
				serveFile(res, req);
				asyncFileStreamer.streamFile(res, req->getUrl());
			})
			.listen(port, [port, root](auto* token) {
				if (token) {
					std::cout << "Serving " << root << " over HTTP a " << port << std::endl;
				}
				}).run();
	}

	std::cout << "Failed to listen to port " << port << std::endl;
}
