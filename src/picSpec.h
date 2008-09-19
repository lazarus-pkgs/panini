/* picSpec.h for pvQt 17 Sep 2008 TKS
*/
#include <QDialog>
#include "ui_PicSpec.h"

class picSpec 
	: public QDialog, public Ui::Dialog
{
	Q_OBJECT
public:
	picSpec( QWidget * parent = 0 );
	
	void setFiles( QStringList files );
	void setFile( int idx, QString name );
	void setDims( QSize wh );
	QSize getDims();
	void setFovs( QSizeF hv );
	QSizeF getFovs();


};
