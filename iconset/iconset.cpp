/****************************************************************************
** iconset.cpp - various graphics handling classes
** Copyright (C) 2001, 2002, 2003  Justin Karneges
**                                 Michail Pishchagin
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,USA.
**
****************************************************************************/

#include"iconset.h"
#include"zip.h"

#include<qfile.h>
#include<qfileinfo.h>
#include<qtimer.h>

#include<qpixmap.h>
#include<qimage.h>
#include<qregexp.h>
#include<qmime.h>
#include<qdom.h>
#include<qfile.h>
#include<qfileinfo.h>
#include<qcstring.h>
#include<qptrvector.h>

#include "anim.h"

//----------------------------------------------------------------------------
// Impix
//----------------------------------------------------------------------------
class Impix::Private
{
public:
	Private()
	{
		pixmap = 0;
		image = 0;
	}

	~Private()
	{
		unload();
	}

	void unload()
	{
		delete pixmap;
		pixmap = 0;
		delete image;
		image = 0;
	}

	QPixmap *pixmap;
	QImage *image;
};

//! \brief Construct a null Impix
//!
//! Constructs an Impix without any image data.
Impix::Impix()
{
	d = new Private;
}

//! \brief Construct an Impix based on a QPixmap
//!
//! Constructs an Impix by making a copy of a QPixmap and creating a QImage from it.
//! \param from - source QPixmap
Impix::Impix(const QPixmap &from)
{
	d = new Private;
	*this = from;
}

//! \brief Construct an Impix based on a QImage
//!
//! Constructs an Impix by making a copy of a QImage and creating a QPixmap from it.
//! \param from - source QImage
Impix::Impix(const QImage &from)
{
	d = new Private;
	*this = from;
}

//! \brief Copy constructor
Impix::Impix(const Impix &from)
{
	d = new Private;
	*this = from;
}

//! Sets the Impix to \param from
Impix & Impix::operator=(const Impix &from)
{
	d->unload();
	if(from.d->pixmap)
		d->pixmap = new QPixmap(*from.d->pixmap);
	if(from.d->image)
		d->image = new QImage(*from.d->image);
	return *this;
}

//! Unloads image data, making it null.
void Impix::unload()
{
	if(isNull())
		return;

	d->unload();
}

//! Destroys the image.
Impix::~Impix()
{
	delete d;
}

bool Impix::isNull() const
{
	return d->image ? true: false;
}

const QPixmap & Impix::pixmap() const
{
	if(!d->pixmap)
		d->pixmap = new QPixmap;
	return *d->pixmap;
}

const QImage & Impix::image() const
{
	if(!d->image)
		d->image = new QImage;
	return *d->image;
}

void Impix::setPixmap(const QPixmap &x)
{
	d->unload();
	d->pixmap = new QPixmap(x);
	d->image = new QImage(x.convertToImage());
}

void Impix::setImage(const QImage &x)
{
	d->unload();
	d->pixmap = new QPixmap;
	d->pixmap->convertFromImage(x);
	d->image = new QImage(x);
}

bool Impix::loadFromData(const QByteArray &ba)
{
	QImage image;
	if ( image.loadFromData( ba ) ) {
		setImage ( image );
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
// Icon
//----------------------------------------------------------------------------

class Icon::Private : public QShared
{
public:
	Private() 
	{ 
		anim = 0;
		activatedCount = 0;
		text.setAutoDelete(true);
	}
	
	~Private()
	{
		unload();
	}
	
	void unload()
	{
		if ( anim )
			delete anim;
		anim = 0;
	}

	QString name;
	QRegExp regExp;
	QDict<QString> text;
	QString sound;
	
	Impix impix;
	Anim *anim;
	
	int activatedCount;
};

Icon::Icon()
: QObject(0, 0)
{
	d = new Private;
}

Icon::~Icon()
{
	if ( d->deref() )
		delete d;
}

Icon::Icon(const Icon &from)
: QObject(0, 0)
{
	d = from.d;
	d->ref();
	//qWarning("Icon: name = %s; regExp = %s", d->name.latin1(), d->regExp.pattern().latin1());
}

Icon & Icon::operator= (const Icon &from)
{
	if ( d->deref() )
		delete d;
		
	d = from.d;
	d->ref();
		
	/*d = new Private;
	
	d->name = from.d->name;
	d->regExp = from.d->regExp;
	d->text = from.d->text;
	d->sound = from.d->sound;
	d->impix = from.d->impix;
	if ( from.d->anim )
		d->anim = new Anim( *from.d->anim );
	d->activatedCount = 0;*/
	
	return *this;
}

bool Icon::isAnimated() const
{
	return d->anim != 0;
}

const QPixmap &Icon::pixmap() const
{
	if ( d->anim )
		return d->anim->framePixmap();
	return d->impix.pixmap();
}

const QImage &Icon::image() const
{
	if ( d->anim )
		return d->anim->frameImage();
	return d->impix.image();
}

const Impix &Icon::impix() const
{
	return d->impix;
}

void Icon::setImpix(const Impix &impix)
{
	d->impix = impix;
	emit pixmapChanged( pixmap() );
}

const Anim *Icon::anim() const
{
	return d->anim;
}

void Icon::setAnim(const Anim &anim)
{
	d->anim = new Anim(anim);
	emit pixmapChanged( pixmap() );
}

const QString &Icon::name() const
{
	return d->name;
}

void Icon::setName(const QString &name)
{
	d->name = name;
}

const QRegExp &Icon::regExp() const
{
	return d->regExp;
}

void Icon::setRegExp(const QRegExp &regExp)
{
	d->regExp = regExp;
}

const QDict<QString> &Icon::text() const
{
	return d->text;
}

void Icon::setText(const QDict<QString> &text)
{
	d->text = text;
}

const QString &Icon::sound() const
{
	return d->sound;
}

void Icon::setSound(const QString &sound)
{
	d->sound = sound;
}

bool Icon::loadFromData(const QByteArray &ba, bool isAnim)
{
	bool ret = false;
	if ( isAnim ) {
		d->anim = new Anim(ba);
		if ( d->anim->numFrames() < 2 ) {
			delete d->anim;
			d->anim = 0;
		}
		else {
			setImpix( d->anim->frame(0) );
			ret = true;
		}
	}
	
	if ( !ret && d->impix.loadFromData(ba) )
		ret = true;
	
	if ( ret )
		emit pixmapChanged( pixmap() );
	
	return ret;
}

void Icon::activated(bool playSound)
{
	d->activatedCount++;
	//qWarning("%-25s Icon::activated count = %d", name().latin1(), d->activatedCount);
	
	if ( playSound && !d->sound.isNull() ) {
		// TODO: insert sound playing code
	}
	
	if ( d->anim ) {
		d->anim->unpause();
			
		d->anim->disconnectUpdate (this, SLOT(animUpdate())); // ensure, that we're connected to signal exactly one time
		d->anim->connectUpdate (this, SLOT(animUpdate()));
	}
}

void Icon::stop()
{
	d->activatedCount--;
	//qWarning("%-25s Icon::stop count = %d", name().latin1(), d->activatedCount);
	
	if ( d->activatedCount <= 0 ) {
		d->activatedCount = 0;
		if ( d->anim ) {
			d->anim->pause();
			d->anim->restart();
		}
	}
}

void Icon::animUpdate()
{
	emit pixmapChanged( pixmap() );
}

//----------------------------------------------------------------------------
// IconsetFactory
//----------------------------------------------------------------------------

class IconsetFactoryPrivate
{
private:
	static QPtrVector<Iconset> *iconsets;

public:
	static void registerIconset(const Iconset *);
	static void unregisterIconset(const Iconset *);

public:
	static const Icon *icon(const QString &name);
	
	friend class IconsetFactory;
};

QPtrVector<Iconset> *IconsetFactoryPrivate::iconsets = 0;

void IconsetFactoryPrivate::registerIconset(const Iconset *i)
{
	if ( !iconsets || iconsets->find(i) < 0 ) {
		if ( !iconsets )
			iconsets = new QPtrVector<Iconset>;
	
		iconsets->resize ( iconsets->count() + 1 );
		iconsets->insert ( iconsets->count(), i );
	}
}

void IconsetFactoryPrivate::unregisterIconset(const Iconset *i)
{
	if ( iconsets && iconsets->find(i) >= 0 ) {
		iconsets->remove ( iconsets->find(i) );
		
		if ( !iconsets->count() ) {
			delete iconsets;
			iconsets = 0;
		}
	}
}

const Icon *IconsetFactoryPrivate::icon(const QString &name)
{
	if ( !iconsets )
		return 0;
	
	const Icon *i = 0;
	for (uint j = 0; j < iconsets->count(); j++) {
		i = iconsets->at(j)->icon(name);
		if ( i )
			break;
	}
	return i;
}

const Icon *IconsetFactory::iconPtr(const QString &name)
{
	const Icon *i = IconsetFactoryPrivate::icon(name);
	if ( !i ) {
		qDebug("WARNING: IconsetFactory::icon(\"%s\"): icon not found", name.latin1());
	}
	return i;
}

const Icon &IconsetFactory::icon(const QString &name)
{
	const Icon *i = iconPtr(name);
	if ( i )
		return *i;
	return Icon();
}

const QStringList IconsetFactory::icons()
{
	QStringList list;
	
	QPtrVector<Iconset> *iconsets = IconsetFactoryPrivate::iconsets;
	uint count = 0;
	if ( iconsets )
		count = iconsets->count();
	for (uint i = 0; i < count; i++) {
		QDictIterator<Icon> it = iconsets->at(i)->iterator();
		for ( ; it.current(); ++it)
			list << (*it)->name();
	}
	
	return list;
	
}

//----------------------------------------------------------------------------
// Iconset
//----------------------------------------------------------------------------

class Iconset::Private : public QShared
{
public:
	Private() 
	{
		name = "Unnamed";
		version = "1.0";
		description = "No description";
		authors << "I. M. Anonymous";
		creation = "XXXX-XX-XX";
		
		list.setAutoDelete(true);
	}

	QByteArray loadData(const QString &fileName, const QString &dir)
	{
		QByteArray ba;

		QFileInfo fi(dir);
		if ( fi.isDir() ) {
			QFile file ( dir + "/" + fileName );
			file.open (IO_ReadOnly);

			ba = file.readAll();
		}
		else if ( fi.extension(false) == "jisp" || fi.extension(false) == "zip" ) {
			UnZip z(dir);
			if ( !z.open() )
				return ba;

			QString name = fi.baseName(true) + "/" + fileName;
			if ( !z.readFile(name, &ba) ) {
				name = "/" + fileName;
				z.readFile(name, &ba);
			}
		}

		return ba;
	}

	static int icon_counter;

	void loadIcon(const QDomElement &i, const QString &dir)
	{
		Icon icon;

		QDict<QString> text, graphic, sound;
		graphic.setAutoDelete(true);
		sound.setAutoDelete(true);

		QString name;
		name.sprintf("icon_%04d", icon_counter++);
		bool isAnimated = false;
		bool isImage = false;

		for(QDomNode n = i.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if ( e.isNull() )
				continue;

			QString tag = e.tagName();
			if ( tag == "text" ) {
				QString lang = e.attribute("xml:lang");
				if ( lang.isEmpty() )
					lang = ""; // otherwise there would be many warnings :(
				text.insert( lang, new QString(e.text()));
			}
			else if ( tag == "graphic" ) {
				graphic.insert( e.attribute("mime"), new QString(e.text()));
			}
			else if ( tag == "sound" ) {
				sound.insert( e.attribute("mime"), new QString(e.text()));
			}
			else if ( tag == "x" ) {
				QString attr = e.attribute("xmlns");
				if ( attr == "name" ) {
					name = e.text();
				}
				else if ( attr == "type" ) {
					if ( e.text() == "animation" )
						isAnimated = true;
					else if ( e.text() == "image" )
						isImage = true;
				}
			}
		}

		icon.setText (text);
		icon.setName (name);

		QStringList graphicMime, soundMime;
		graphicMime << "image/png"; // first item have higher priority than latter
		graphicMime << "video/x-mng";
		graphicMime << "image/gif";
		graphicMime << "image/bmp";
		graphicMime << "image/x-xpm";
		//graphicMime << "image/svg+xml";
		graphicMime << "image/jpeg";

		soundMime << "audio/x-wav"; // same here
		soundMime << "audio/x-ogg";
		soundMime << "audio/x-mp3";
		soundMime << "audio/x-midi";

		{
			QStringList::Iterator it = graphicMime.begin();
			for ( ; it != graphicMime.end(); ++it) {
				if ( graphic[*it] && !graphic[*it]->isNull() ) {
					bool anim = isAnimated;

					// if format supports animations, then load graphic as animation, and
					// if there is only one frame, then later it would be converted to single Impix
					if ( !anim && !isImage &&
					     ( *it == "image/gif"
					//    || *it == "image/png"
					    || *it == "video/x-mng" )) {
						anim = true;
					}
					if ( icon.loadFromData( loadData(*graphic[*it], dir), anim ) )
						break;
				}
			}
		}

		{
			QFileInfo fi(dir);
			QStringList::Iterator it = soundMime.begin();
			for ( ; it != soundMime.end(); ++it) {
				if ( sound[*it] && !sound[*it]->isNull() ) {
					if ( !fi.isDir() ) { // it is a .zip then
						// TODO: write unpacking code here
						// probably it should create directory with
						// archive name in the data directory, and
						// unpack there...
					}
					else {
						icon.setSound ( dir + "/" + *sound[*it] );
						break;
					}
				}
			}
		}

		if ( text.count() )
		{	// construct RegExp
			QString regexp;
			QDictIterator<QString> it( text );
			for ( ; it.current(); ++it ) {
				//QString s = **it;

				/*char symbols[] = { // these symbols needs to be prefixed with backslash
					'(', ')', '|', '$', '{', '}',
					'[', ']', '*', '.', '%', 0
				};

				for (int j = 0; symbols[j]; j++)
					s.replace (symbols[j], QString("\\%1").arg(symbols[j]));
				*/
				if ( !regexp.isEmpty() )
					regexp += '|';

				regexp += QRegExp::escape(**it); //s;
			}

			// make sure there is some form of whitespace on at least one side of the text string
			regexp = QString("(\\b(%1))|((%2)\\b)").arg(regexp).arg(regexp);
			icon.setRegExp ( regexp );
		}

		list.replace (name, new Icon(icon));
	}

	bool load(const QDomDocument &doc, const QString dir)
	{
		QDomElement base = doc.documentElement();
		if ( base.tagName() != "icondef" )
			return false;

		for(QDomNode node = base.firstChild(); !node.isNull(); node = node.nextSibling()) {
			QDomElement i = node.toElement();
			if( i.isNull() )
				continue;

			QString tag = i.tagName();
			if ( tag == "name" ) {
				name = i.text();
			}
			else if ( tag == "version" ) {
				version = i.text();
			}
			else if ( tag == "description" ) {
				description = i.text();
			}
			else if ( tag == "author" ) {
				QString name = i.text();
				if ( !i.attribute("email").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;Email: %2").arg(name).arg( i.attribute("email") );
				if ( !i.attribute("jid").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;JID: %2").arg(name).arg( i.attribute("jid") );
				if ( !i.attribute("www").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;WWW: %2").arg(name).arg( i.attribute("www") );
				authors += name;
			}
			else if ( tag == "creation" ) {
				creation = i.text();
			}
			else if ( tag == "icon" ) {
				loadIcon (i, dir);
			}
			else if ( tag == "x" ) {
				info.insert( i.attribute("xmlns"), new QString(i.text()));
			}
		}

		return true;
	}

	QString name, version, description, creation, filename;
	QStringList authors;
	QDict<Icon> list;
	QDict<QString> info;
	//Icon nullIcon;
};

int Iconset::Private::icon_counter = 0;

Iconset::Iconset()
{
	d = new Private;
}

Iconset::Iconset(const Iconset &from)
{
	d = from.d;
	d->ref();
}

Iconset::~Iconset()
{
	if ( d->deref() )
		delete d;

	IconsetFactoryPrivate::unregisterIconset(this);
}

Iconset &Iconset::operator=(const Iconset &from)
{
	clear();
	QDictIterator<Icon> it( from.d->list );
	for ( ; it.current(); ++it)
		d->list.insert(it.currentKey(), new Icon(*it.current()));
		
	d->name = from.d->name;
	d->version = from.d->version;
	d->description = from.d->description;
	d->authors = from.d->authors;
	d->creation = from.d->creation;
	
	return *this;
}

Iconset &Iconset::operator+=(const Iconset &i)
{
	QDictIterator<Icon> it( i.d->list );
	for ( ; it.current(); ++it)
		d->list.insert(it.currentKey(), new Icon(*it.current()));
	return *this;
}

void Iconset::clear()
{
	d->list.clear();
}

uint Iconset::count() const
{
	return d->list.count();
}

bool Iconset::load(const QString &dir)
{
	QByteArray ba;
	ba = d->loadData ("icondef.xml", dir);
	if ( !ba.isEmpty() ) {
		QDomDocument doc;
		if ( doc.setContent(ba, false) ) {
			if ( d->load(doc, dir) ) {
				d->filename = dir;
				return true;
			}
		}
		else
			qWarning("Iconset::load: Failed to load iconset: icondef.xml is invalid XML");
	}
	return false;
}

const Icon *Iconset::icon(const QString &name) const
{
	if ( d->list.isEmpty() )
		return 0;
	
	return d->list.find(name);
}

void Iconset::setIcon(const QString &name, const Icon &icon)
{
	d->list.replace (name, new Icon(icon));
}

void Iconset::removeIcon(const QString &name)
{
	d->list.remove (name);
}

const QString &Iconset::name() const
{
	return d->name;
}

const QString &Iconset::version() const
{
	return d->version;
}

const QString &Iconset::description() const
{
	return d->description;
}

const QStringList &Iconset::authors() const
{
	return d->authors;
}

const QString &Iconset::creation() const
{
	return d->creation;
}

QDictIterator<Icon> Iconset::iterator() const
{
	QDictIterator<Icon> it( d->list );
	return it;
}

const QString &Iconset::fileName() const
{
	return d->filename;
}

void Iconset::setFileName(const QString &f)
{
	d->filename = f;
}

const QDict<QString> Iconset::info() const
{
	return d->info;
}

void Iconset::setInfo(const QDict<QString> &i)
{
	d->info = i;
}

QMimeSourceFactory *Iconset::createMimeSourceFactory() const
{
	QMimeSourceFactory *m = new QMimeSourceFactory;

	QDictIterator<Icon> it( d->list );
	for ( ; it.current(); ++it)
		m->setImage(it.currentKey(), it.current()->image());
	
	return m;
}

void Iconset::addToFactory() const
{
	IconsetFactoryPrivate::registerIconset(this);
}

void Iconset::removeFromFactory() const
{
	IconsetFactoryPrivate::unregisterIconset(this);
}
