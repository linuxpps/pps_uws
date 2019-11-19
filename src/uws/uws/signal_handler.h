/**
 * @file signal_handler.h
 * @brief signal_handler defines
 */

#pragma once

#include <iostream>
#include <signal.h>
#include <time.h>

static bool m_flag;
static sigset_t m_wait_mask = { 0 };
static struct timespec m_time = { 0 };
class CSignalHandler {
public:
	static void init() {
		m_flag = true;
		try {
			//signal(SIGKILL, SIG_IGN);
			sigemptyset(&m_wait_mask);
			sigaddset(&m_wait_mask, SIGINT);
			sigaddset(&m_wait_mask, SIGQUIT);
			sigaddset(&m_wait_mask, SIGKILL);
			sigaddset(&m_wait_mask, SIGTERM);
			pthread_sigmask(SIG_BLOCK, &m_wait_mask, 0);
		}
		catch (std::exception & e) {
			std::cerr << "exception: " << e.what() << std::endl;
		}
		m_time.tv_sec = 0;
		m_time.tv_nsec = 0;
	}
	static bool wait() {

		//std::cout << "wait signal" << std::endl;
		try {
			siginfo_t siginfo = { 0 };
			int sig = sigtimedwait(&m_wait_mask, &siginfo, &m_time);
			switch (sig) {
			case SIGINT:
			case SIGQUIT:
			case SIGKILL:
			case SIGTERM:
			{
				m_flag = false;
				//std::cerr << "handled signal:" << sig << std::endl;
			}
			break;
			default:
			{
				//std::cerr << "unhandled signal:" << sig << std::endl;
			}
			break;
			}
		}
		catch (std::exception & e) {
			std::cerr << "exception: " << e.what() << std::endl;
		}
		return m_flag;
	}
};

#define G_SIGNAL_INIT CSignalHandler::init
#define G_SIGNAL_WAIT CSignalHandler::wait
