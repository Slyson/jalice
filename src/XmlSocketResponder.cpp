#include "XmlSocketResponder.h"

#include "Kernel.h"
#include "SocketHandler.h"
#include "TokenProxyListener.h"
#include "Logger.h"

#include <iostream>
#include <stdlib.h>
//#ifdef __BEOS__
// #include <sstream>
// #define stringstream strstream
//#else
 #include <strstream>
//#endif

using namespace std;

XmlSocketProcessor::XmlSocketProcessor() {
    ;
}

string XmlSocketProcessor::process(Match *m, PElement e, Responder *r, const string &id) {
    port = atoi(Kernel::process(m, e->getChild("port"), r, id).c_str());
    
    server = new ServerSocket(port);
    server->setServerListener(this);
    server->init();
    cout << "Starting up an XML Socket Server on port " << port << endl;
    SocketHandler::runLoop();    
    return "";
}

void XmlSocketProcessor::shutdown(const string &msg) {
    cout << "Shutting down XML Socket Server on port " << port << endl;
    cout << "Reason: " << msg << endl;
}

void XmlSocketProcessor::awaitingClient(Socket *client) {
    //    Temporary measure...
    client->setListener(new XmlSocketResponder(client));
//?    client->process();
    client->getListener()->connected();
}

XmlSocketResponder::XmlSocketResponder(Socket *c) {
    client = c;
    botName = Kernel::respond("BOT NAME", "system");
    parser = new Parser();
}

string XmlSocketResponder::respond(Match *, PElement, const string &) {
    return "";
}

void XmlSocketResponder::recv(string &s) {
    strstream strs;
    strs << s << endl;
    PElement root = parser->parse(strs, "meh");
    if (root == NULL) {
        return;
    }
    if (root->getTagname() != "message") {
    //    send("<message>Error processing message</message>");
        return;
    }
    if (root->getChild("#text") == NULL) {
        return;
    }
    string text = root->getChild("#text")->getText();
    string reply = Kernel::respond(text, "xmlsocket");
    send("<message>" + reply + "</message>");
}

void XmlSocketResponder::connected() {
    string connectString = "<connect><botname>" + botName + "</botname>";
    connectString += "<message>" + Kernel::respond("CONNECT", client->getPeerName()) + "</message></connect>";
    send(connectString);
}

void XmlSocketResponder::disconnected(const string &msg) {
    cout << "Notice: " << client->getPeerName() << " disconnected (" << msg << ")" << endl;
}

void XmlSocketResponder::send(const string &s) {
    if (client == NULL) {
        return;
    }
    client->write(s, true);
}