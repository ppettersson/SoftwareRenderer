#include "SoftwareRenderer.h"
#include "TestSuite.h"

int main(int argc, char **argv)
{
	if (!Init(1024, 768))
		return -1;

	if (!InitTestSuite())
		return -1;

	for (;;)
	{
		// Clear screen to non-black to detect pixel errors easier.
		Clear(kColorLightGrey);

		RunTestSuite();

		// Swap buffers.
		if (!PresentFrame())
			break;
	}

	QuitTestSuite();
	Quit();
	return 0;
}
