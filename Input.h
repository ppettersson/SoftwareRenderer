#ifndef INPUT_H
#define INPUT_H

enum KeyCode
{
	kKey_Left,
	kKey_Right,

	kKey_Last
};

bool KeyPressed(int code);
bool KeyDown(int code);

#endif // INPUT_H
