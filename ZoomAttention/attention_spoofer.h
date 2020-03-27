#pragma once
#include "pch.h"
#include "detour.h"

class attention_spoofer {
public:
	attention_spoofer(): m_meeting_window(nullptr), m_get_focus_hook(nullptr) {};

	void init_hooks();
	void destroy_hooks();

	void find_correct_window(HWND active);
	HWND override_window(HWND actual_window);

	detour_hook* m_get_focus_hook;
	detour_hook* m_get_foreground_window_hook;
	detour_hook* m_get_active_window_hook;

private:
	HWND m_meeting_window;
	std::mutex m_stats_mtx;
};
extern attention_spoofer* g_spoofer;

HWND __stdcall hooked_get_focus();
HWND __stdcall hooked_get_foreground_window();
HWND __stdcall hooked_get_active_window();