/* picSpec.h for pvQt 17 Sep 2008 TKS
*/
#include <QWidget>
#include "ui_PicSpec.h"

class picSpec : public QWidget, public Ui::Dialog
{
	Q_OBJECT
public:
	picSpec( QString startDir = "" );

};