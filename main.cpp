#include <iostream>
#include <thread>
#include <Windows.h>
#include <tuple>
#include <string>

CONST float SLEEP_DURATION_MS = 8;

HWND createWindow(LPCWSTR title, int x, int y, int w, int h);

struct MetaData {
	int screenWidth;
	int screenHeight;
};

MetaData get_metadata() {
	MetaData meta;
	meta.screenWidth = GetSystemMetrics(SM_CXSCREEN);
	meta.screenHeight = GetSystemMetrics(SM_CYSCREEN);
	return meta;
}

struct Ball {
	HWND window;
	std::pair<float, float> pos;		// px
	std::pair<float, float> velocity;	//px per second
};

Ball createBall(std::pair<float, float> pos, std::pair<float, float> velocity) {
	return Ball{
		createWindow(L"Ball", (int)pos.first, (int)pos.second, 100, 100),
		pos,
		velocity
	};
}

void spawnMessageBox()
{
	MessageBox(0, L"temp", L"temp", MB_OK);

	exit(0);
}

// Create a message box, then update its title to `title`
HWND createWindow(LPCWSTR title, int x, int y, int w, int h)
{
	std::thread t(spawnMessageBox);
	t.detach();
	Sleep(32); //wait some time for the thread and the window to open.
	HWND ret = FindWindow(0, L"temp");

	// Set parameters
	SetWindowText(ret, title);
	SetWindowPos(ret, HWND_TOP, x, y, w, h, NULL);

	return ret;
}

void update_ball_velocity(Ball* ball, HWND* paddle, MetaData meta) {
	const float TO_RADS = (3.14159 / 180.0);
	const float MAX_ANGLE = 60 * (3.14159 / 180.0);
	RECT ballRect;
	RECT paddleRect;
	GetWindowRect((*ball).window, &ballRect);
	GetWindowRect(*paddle, &paddleRect);

	// COLLIDES WITH PADDLE
	if (ballRect.bottom >= paddleRect.top // collision in y-dimension
		&& ballRect.top < paddleRect.top // Not entirely below paddle
		&& (ballRect.right >= paddleRect.left && ballRect.left <= paddleRect.right) // collision in x-dimension
		&& ball->velocity.second > 0 // Prevent ball from bouncing inside the paddle indefinitely
		) {
		
		float hit_pos = (ballRect.left + ballRect.right) / 2.0f - (paddleRect.left + paddleRect.right) / 2.0f; // Point of collision relative to the center of the paddle
		float paddle_center = (paddleRect.right - paddleRect.left) / 2.0;
		float norm_hit_pos = hit_pos / paddle_center; //between -1 and 1

		float angle = norm_hit_pos * MAX_ANGLE;
		std::cout << angle << "\n";

		float speed = sqrt(ball->velocity.first * ball->velocity.first + ball->velocity.second * ball->velocity.second);
		ball->velocity.first = speed * sin(angle);
		ball->velocity.second = -speed * cos(angle);
		return;
	}

	// === COLLIDES WITH EDGE OF SCREEN ===
	// LEFT
	if (ballRect.left <= 0) {
		ball->velocity.first *= -1;
	}

	// RIGHT
	if (ballRect.right >= meta.screenWidth) {
		ball->velocity.first *= -1;
	}

	// TOP
	if (ballRect.top <= 0) {
		ball->velocity.second *= -1;
	}

	// Off bottom
	if (ballRect.top >= meta.screenHeight -1) {
		*ball = createBall(std::make_pair(700, 700), std::make_pair(-1000, 350));
	}
	return;
}

void update_ball_position(Ball* ball) {
	ball->pos.first += ball->velocity.first * (SLEEP_DURATION_MS / 1000.0);
	ball->pos.second += ball->velocity.second * (SLEEP_DURATION_MS / 1000.0);
}

int main()
{
	MetaData meta = get_metadata();
	Ball ball = createBall(std::make_pair(40, meta.screenHeight/3), std::make_pair(1000, 500));
	HWND paddle = createWindow(L"Paddle", meta.screenWidth / 2, meta.screenHeight * 9 / 10, 500, 100);

	int i = 0;
	while (true)
	{
		update_ball_velocity(&ball, &paddle, meta);
		update_ball_position(&ball);

		//change window border
		if (GetAsyncKeyState('Q') & 1)
		{
			std::cout << "Pressed Q\n";
			auto style = GetWindowLongPtrA(ball.window, GWL_STYLE);
			style ^= WS_BORDER;
			SetWindowLongPtrA(ball.window, GWL_STYLE, style);
		}

		//diasble window
		if (GetAsyncKeyState('W') & 1)
		{
			std::cout << "Pressed W\n";
			auto style = GetWindowLongPtrA(ball.window, GWL_STYLE);
			style ^= WS_DISABLED;
			SetWindowLongPtrA(ball.window, GWL_STYLE, style);
		}

		//get window size
		if (GetAsyncKeyState('E') & 1)
		{
			std::cout << "Pressed E\n";

			RECT r = {};
			GetWindowRect(ball.window, &r);

			std::cout << "Window width: " << r.right - r.left <<
				" window height: " << r.bottom - r.top << '\n';
		}

		RECT ball_rect;
		GetWindowRect(ball.window, &ball_rect);
		SetWindowPos(ball.window, HWND_TOPMOST, ball.pos.first, ball.pos.second, 100, 100, NULL);
		Sleep(SLEEP_DURATION_MS);

	}

	return 0;
}