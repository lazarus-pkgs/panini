/* About.cpp for pvQt
*/

#include "About.h"
#include "pvQtVersion.h"

pvQtAbout::pvQtAbout( QWidget * parent )
: QDialog( parent )
{
	setupUi( this );
	VersionLabel->setText( QString(pvQtVersion) );
}

void pvQtAbout::setInfo( QString info )
{
	InfoLabel->setText( info );
}


