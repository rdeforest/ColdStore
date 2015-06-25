from gtk import *
from gnome.ui import *

win = GtkWindow()
win.connect('destroy', mainquit)
win.set_title('Canvas test')

canvas = GnomeCanvas()
canvas.set_size(300, 300)
win.add(canvas)
canvas.show()

canvas.root()

group=GnomeCanvasGroup( canvas.root() );
rect = GnomeCanvasRect(group);
rect.set_usize(1,1,10,10);
win.show()

mainloop()

