#include "pch.h"
#include "attention_spoofer.h"

attention_spoofer* g_spoofer = nullptr;

void attention_spoofer::init_hooks() {
	printf("Placing hooks...\n");

	HMODULE user32 = GetModuleHandleA("user32.dll");
	if (!user32) {
		printf("Fatal error: could not find user32.dll\n");
		return;
	}

	printf("user32.dll loaded at base 0x%X\n", uintptr_t(user32));

	void* get_focus = GetProcAddress(user32, "GetFocus");
	if (!get_focus) {
		printf("Fatal error: could not find user32!GetFocus\n");
		return;
	}
	m_get_focus_hook = new detour_hook(get_focus, hooked_get_focus, 9);
	printf("Hooked user32!GetFocus: user32.dll + 0x%X (%X)\n", uintptr_t(get_focus) - uintptr_t(user32), uintptr_t(get_focus));

	void* get_foreground_window = GetProcAddress(user32, "GetForegroundWindow");
	if (!get_foreground_window) {
		printf("Fatal error: could not find user32!GetForegroundWindow\n");
		return;
	}
	m_get_foreground_window_hook = new detour_hook(get_foreground_window, hooked_get_foreground_window, 6);
	printf("Hooked user32!GetForegroundWindow: user32.dll + 0x%X (%X)\n", uintptr_t(get_foreground_window) - uintptr_t(user32), uintptr_t(get_foreground_window));

	void* get_active_window = GetProcAddress(user32, "GetActiveWindow");
	if (!get_active_window) {
		printf("Fatal error: could not find user32!GetActiveWindow\n");
		return;
	}
	m_get_active_window_hook = new detour_hook(get_active_window, hooked_get_active_window, 5);
	printf("Hooked user32!GetActiveWindow: user32.dll + 0x%X (%X)\n", uintptr_t(get_active_window) - uintptr_t(user32), uintptr_t(get_active_window));
}

void attention_spoofer::destroy_hooks() {
	printf("Destroying hooks...\n");

	if (m_get_focus_hook)
		delete m_get_focus_hook;

	if (m_get_foreground_window_hook)
		delete m_get_foreground_window_hook;

	if (m_get_active_window_hook)
		delete m_get_active_window_hook;

	Sleep(200);
}

void attention_spoofer::find_correct_window(HWND active) {
	// ignore the call if we already found the window
	if (m_meeting_window)
		return;

	char window_name[MAX_PATH];
	DWORD process_id = 0;
	GetWindowTextA(active, window_name, MAX_PATH);
	GetWindowThreadProcessId(active, &process_id);

	// make sure the window is the correct one we want to be spoofing to, if it is: save it
	if (process_id == GetCurrentProcessId() && strstr(window_name, "Zoom Meeting")) {
		m_meeting_window = active;
		printf("Found window %i -> %s (PID: %i)\nStarting focus spoof...\n", active, window_name, process_id);
	}
}

HWND attention_spoofer::override_window(HWND actual_window) {
	if (m_meeting_window)
		return m_meeting_window; // override actual window if we have found it

	return actual_window;
}

HWND __stdcall hooked_get_focus() {
	HWND ret = 0;

	// call original
	if (g_spoofer->m_get_focus_hook) {
		auto fn = g_spoofer->m_get_focus_hook->get_orig<HWND(__stdcall*)()>();
		ret = fn();
	}

	// see if we can find the current "actual" window we want to spoof to
	g_spoofer->find_correct_window(ret);

	// try to override the return value if we are properly initialized
	return g_spoofer->override_window(ret);
}

HWND __stdcall hooked_get_foreground_window() {
	HWND ret = 0;

	// call original
	if (g_spoofer->m_get_foreground_window_hook) {
		auto fn = g_spoofer->m_get_foreground_window_hook->get_orig<HWND(__stdcall*)()>();
		ret = fn();
	}

	// see if we can find the current "actual" window we want to spoof to
	g_spoofer->find_correct_window(ret);

	// try to override the return value if we are properly initialized
	return g_spoofer->override_window(ret);
}

HWND __stdcall hooked_get_active_window() {
	HWND ret = 0;

	// call original
	if (g_spoofer->m_get_active_window_hook) {
		auto fn = g_spoofer->m_get_active_window_hook->get_orig<HWND(__stdcall*)()>();
		ret = fn();
	}

	// see if we can find the current "actual" window we want to spoof to
	g_spoofer->find_correct_window(ret);

	// try to override the return value if we are properly initialized
	return g_spoofer->override_window(ret);
}