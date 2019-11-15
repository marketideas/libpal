#pragma once
#include "config.h"
#include "pal_config.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

#define QUOTE(str) #str
#define STRINGIFY(str) QUOTE(str)

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <grp.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>


// base includes

#include <Poco/Buffer.h>
#include <Poco/MemoryStream.h>

#include <Poco/AutoPtr.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Random.h>
#include <Poco/RandomStream.h>

#include <Poco/SignalHandler.h>

#include <Poco/Timestamp.h>

#include <Poco/DateTime.h>
#include <Poco/Timer.h>
#include <Poco/Stopwatch.h>

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

#include <Poco/DateTimeParser.h>

#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"

#include <Poco/LocalDateTime.h>
#include <Poco/Environment.h>
#include <Poco/RegularExpression.h>
#include <Poco/HashMap.h>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#include <Poco/MemoryStream.h>

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NamedNodeMap.h>

#include <Poco/DOM/AutoPtr.h>
#include <Poco/SAX/InputSource.h>

#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>

#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/PrintHandler.h>
#include <Poco/JSON/Stringifier.h>

#ifdef USE_NET
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/NetException.h>

#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherImpl.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Crypto/CipherKeyImpl.h>
#include <Poco/Crypto/Crypto.h>
#include <Poco/Crypto/CryptoStream.h>
#include <Poco/Crypto/CryptoTransform.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Crypto/OpenSSLInitializer.h>
#include <Poco/Crypto/RSACipherImpl.h>
#include <Poco/Crypto/RSADigestEngine.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/Crypto/RSAKeyImpl.h>
#include <Poco/Crypto/X509Certificate.h>
#endif

#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Task.h>
#include <Poco/TaskNotification.h>
#include <Poco/TaskManager.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/LineEndingConverter.h>


//#include <Poco/CppParser/Enum.h>

#include <Poco/NullStream.h>


#ifdef USE_NET
// Network includes
#include <Poco/Net/Net.h>
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"

#include <Poco/Net/TCPServer.h>
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SecureServerSocket.h"

#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"

#include "Poco/Net/SocketStream.h"

#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/URI.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequestImpl.h>

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>

#endif

#include "Poco/Runnable.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Activity.h"

#include "Poco/Thread.h"
#include "Poco/ThreadPool.h"

#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/NotificationCenter.h>
#include <Poco/NotificationQueue.h>
#include <Poco/Notification.h>
#include <Poco/Observer.h>
#include <Poco/NObserver.h>


using namespace std;
using namespace Poco;
#ifdef USE_NET
using namespace Poco::Net;
#endif
//using namespace Poco::Crypto;

using Poco::format;
using Poco::NotFoundException;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Task;
using Poco::TaskManager;
using Poco::DateTimeFormatter;
using Poco::RegularExpression;
using Poco::Message;
using Poco::AutoPtr;

typedef std::map<std::string,std::string> TValueMap;

#ifndef ENABLE_SSL
#define SecureServerSocket ServerSocket
#endif

