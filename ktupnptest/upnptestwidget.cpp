#include <util/log.h>
#include "upnpmcastsocket.h"
#include "upnptestwidget.h"
#include "upnprouter.h"

using namespace bt;
using namespace kt;

UPnPTestWidget::UPnPTestWidget(QWidget* parent) : QWidget(parent)
{
	setupUi(this);
	connect(m_find_routers,SIGNAL(clicked()),this,SLOT(findRouters()));
	connect(m_forward,SIGNAL(clicked()),this,SLOT(doForward()));
	connect(m_undo_forward,SIGNAL(clicked()),this,SLOT(undoForward()));
	mcast_socket = 0;
	router = 0;

	m_forward->setEnabled(false);
	m_undo_forward->setEnabled(false);
	m_port->setEnabled(false);
	m_protocol->setEnabled(false);

	AddLogMonitor(this);
}

UPnPTestWidget::~UPnPTestWidget()
{
	if (mcast_socket)
		delete mcast_socket;
}

void UPnPTestWidget::doForward()
{
	QString proto = m_protocol->currentText();
	bt::Uint16 port = m_port->value();
	Out(SYS_GEN|LOG_DEBUG) << "Forwarding port " << port << " (" << proto << ")" << endl;
	net::Port p(port,proto == "UDP" ? net::UDP : net::TCP,true);
	router->forward(p);
}
	
void UPnPTestWidget::undoForward()
{
	QString proto = m_protocol->currentText();
	bt::Uint16 port = m_port->value();
	Out(SYS_GEN|LOG_DEBUG) << "Unforwarding port " << port << " (" << proto << ")" << endl;
	net::Port p(port,proto == "UDP" ? net::UDP : net::TCP,true);
	router->undoForward(p);
}

void UPnPTestWidget::findRouters()
{
	Out(SYS_GEN|LOG_DEBUG) << "Searching for routers ..." << endl;
	if (!mcast_socket)
	{
		mcast_socket = new UPnPMCastSocket();
		connect(mcast_socket,SIGNAL(discovered(kt::UPnPRouter*)),this,SLOT(discovered(kt::UPnPRouter*)));
	}

	mcast_socket->discover();
}

void UPnPTestWidget::discovered(kt::UPnPRouter* r)
{
	router = r;
	m_router->setText(router->getServer());
	m_forward->setEnabled(true);
	m_undo_forward->setEnabled(true);
	m_port->setEnabled(true);
	m_protocol->setEnabled(true);
	router->setVerbose(true);
}

void UPnPTestWidget::message(const QString & line, unsigned int arg)
{
	m_text_output->append(line);
}

#include "upnptestwidget.moc"
