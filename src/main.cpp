
#include "palPoco.h"
#include "baseapp.h"

int main(int argc, char **argv)
{
	int x;
	int res = 0;
	try
	{
		Poco::AutoPtr<PAL_BASEAPP> pApp = appFactory();

#ifdef SERVERAPP
		x = pApp->run(argc, argv);
#else
		pApp->init(argc, argv);
		x = pApp->run();
#endif
		//cout << "main out" << endl;
		return (x);
	}
	catch (Poco::Exception &e)
	{
		cout << "main() Poco::Exception: " << e.displayText() << endl;
		cout.flush();
		res = -1;

	}
	catch (std::exception &e)
	{
		cout << "main() std::exception: " << e.what() << endl;
		cout.flush();
		res = -1;

	}
	catch (...)
	{
		cout << "main() unhandled exception" << endl;
		cout.flush();
		res = -1;
	}
	return (res);
}