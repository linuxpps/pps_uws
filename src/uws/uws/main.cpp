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

#include <unordered_map>

std::string test_chart()
{
	std::string X = "";
	std::string Y = "";
	std::string T = "";
	std::unordered_map<std::string, std::string> umap = {
		{"20191119", ""},
		{"20191118", ""},
		{"20191115", ""},
		{"20191114", ""},
	};
	X.append("[");
	Y.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./dce.csv");
		//file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/dce.csv");
		file_reader(data, "./" + it.first + "/dce.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "合约代码：(.*?),Date：(.*?),\r\n";
			std::string pattern2 = "总计,(.*?),(.*?),\r\n";
			std::vector<std::vector<std::string>> svv1;
			std::vector<std::vector<std::string>> svv2;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			printf("flag = %d\n", flag);
			if (svv1.size())
			{
				T = svv1.at(0).at(0).c_str();
				printf("%s,%s\n", svv1.at(0).at(0).c_str(), svv1.at(1).at(0).c_str());
			}
			flag = string_regex_find(result, svv2, out.c_str(), pattern2);
			printf("flag = %d\n", flag);
			if (svv2.size())
			{
				printf("%s,%s\n", svv2.at(0).at(0).c_str(), svv2.at(1).at(0).c_str());
			}
			it.second = svv2.at(1).at(0);
			X.append("'").append(it.first).append("',");
			Y.append("'").append(it.second).append("',");
			T = svv1.at(0).at(0).c_str();
		}
	}

	*X.rbegin() = ']';
	*Y.rbegin() = ']';
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, X, "BBBBBB");
	string_replace_all(temp, Y, "AAAAAA");
	file_writer(temp, T + ".html");
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
					res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(test_chart().c_str());
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
