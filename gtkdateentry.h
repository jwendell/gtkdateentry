/*
 * This is a proptotype of a DateEntry widget
 * It's based on the widget found in HomeBank software, so:
 *     Copyright (C) 1995-2013 Maxime DOYEN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Original, Gtk+ 2.0 widget: Maxime DOYEN
 *      Jonh Wendell <jonh.wendell@gmail.com>
 */

#ifndef __GTK_DATE_ENTRY_H__
#define __GTK_DATE_ENTRY_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_DATE_ENTRY            (gtk_date_entry_get_type ())
#define GTK_DATE_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntry))
#define GTK_DATE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass)
#define GTK_IS_DATE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATE_ENTRY))
#define GTK_IS_DATE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATE_ENTRY))
#define GTK_DATE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass))

typedef struct _GtkDateEntry            GtkDateEntry;
typedef struct _GtkDateEntryClass       GtkDateEntryClass;
typedef struct _GtkDateEntryPrivate     GtkDateEntryPrivate;

struct _GtkDateEntry
{
  GtkGrid parent_instance;
  GtkDateEntryPrivate *priv;
};

struct _GtkDateEntryClass
{
  GtkGridClass parent_class;

  /* signals */
  void     (* changed)          (GtkDateEntry *dateentry);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GType           gtk_date_entry_get_type (void);

GtkWidget      *gtk_date_entry_new (void);

void            gtk_date_entry_set_date (GtkDateEntry *date_entry, const GDate *date);
const GDate    *gtk_date_entry_get_date (GtkDateEntry *date_entry);

void            gtk_date_entry_set_mindate (GtkDateEntry *date_entry, const GDate *mindate);
const GDate    *gtk_date_entry_get_mindate (GtkDateEntry *date_entry);

void            gtk_date_entry_set_maxdate (GtkDateEntry *date_entry, const GDate *maxdate);
const GDate    *gtk_date_entry_get_maxdate (GtkDateEntry *date_entry);

G_END_DECLS

#endif /* __GTK_DATE_ENTRY_H__ */
