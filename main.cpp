#include <iostream>
#include <thread>
#include <Windows.h>
#include <tuple>
#include <string>
#include <vector>

CONST float SLEEP_DURATION_MS = 8;
const int TARGETS_PER_ROW = 15;
const int NUM_ROWS = 6;

std::vector<int> death_signals(TARGETS_PER_ROW * NUM_ROWS, 0);
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

void spawnMessageBox(){
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

void update_ball_velocity(Ball* ball, HWND* paddle, std::vector<HWND> targets, MetaData meta) {
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

	// COLLIDES WITH TARGET

	for (int i = 0; i < targets.size(); i++) {
		HWND target = targets[i];
		RECT targetRect;
		GetWindowRect(target, &targetRect);
		bool collides_from_bottom = ballRect.top <= targetRect.bottom && ballRect.bottom >= targetRect.top;
		bool collides_from_top = ballRect.bottom >= targetRect.top && ballRect.top <= targetRect.bottom;
		if (collides_from_bottom || collides_from_top) {
			ball->velocity.second *= -1;
			//death_signals[i] = 1;
			return;
		}
	}

	// === COLLIDES WITH EDGE OF SCREEN ===
	// LEFT
	if (ballRect.left <= 0) {
		ball->velocity.first *= -1;
		return;
	}

	// RIGHT
	if (ballRect.right >= meta.screenWidth) {
		ball->velocity.first *= -1;
		return;
	}

	// TOP
	if (ballRect.top <= 0) {
		ball->velocity.second *= -1;
		return;
	}

	// Off bottom
	if (ballRect.top >= meta.screenHeight -1) {
		*ball = createBall(std::make_pair(700, 700), std::make_pair(-1000, 350));
		return;
	}
	return;
}

void update_ball_position(Ball* ball) {
	ball->pos.first += ball->velocity.first * (SLEEP_DURATION_MS / 1000.0);
	ball->pos.second += ball->velocity.second * (SLEEP_DURATION_MS / 1000.0);
}

std::vector<HWND> create_targets(MetaData meta) {
	const float PERCENTAGE_OF_SCREEN_HEIGHT = 0.03;
	std::vector<HWND> target_vector;
	target_vector.reserve(TARGETS_PER_ROW * NUM_ROWS);

	int h_margin = 0;		// On either side
	int v_margin = 100;		// On top
	int target_w = (meta.screenWidth - 2*h_margin) / TARGETS_PER_ROW;
	int target_h = meta.screenHeight * PERCENTAGE_OF_SCREEN_HEIGHT;

	for (int i = 0; i < TARGETS_PER_ROW; i++) {
		for (int j = 0; j < NUM_ROWS; j++) {
			HWND target = createWindow(L"Target", h_margin + target_w * i, v_margin + target_h * j, target_w, target_h);
			target_vector.push_back(target);
		}
	}

	return target_vector;
}

int main()
{
	MetaData meta = get_metadata();
	Ball ball = createBall(std::make_pair(40, meta.screenHeight/3), std::make_pair(1000, 500));
	HWND paddle = createWindow(L"Paddle", meta.screenWidth / 2, meta.screenHeight * 9 / 10, 500, 100);
	std::vector<HWND> target_array = create_targets(meta);

	while (true)
	{
		update_ball_velocity(&ball, &paddle, target_array, meta);
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