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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gtkdateentry.h"

enum {
  CHANGED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_DATE,
  PROP_MINDATE,
  PROP_MAXDATE
};

struct _GtkDateEntryPrivate
{
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *popwin;
  GtkWidget *calendar;

  GDate *date;
  GDate *mindate, *maxdate;

  gulong day_selected_signal;
};

static guint dateentry_signals[LAST_SIGNAL] = {0,};

G_DEFINE_TYPE (GtkDateEntry, gtk_date_entry, GTK_TYPE_GRID)

// todo:finish this
// this is to be able to seizure d or d/m or m/d in the gtkdateentry

/* order of these in the current locale */
static GDateDMY dmy_order[3] = 
{
   G_DATE_DAY, G_DATE_MONTH, G_DATE_YEAR
};

struct _GDateParseTokens {
  gint num_ints;
  gint n[3];
  guint month;
};

typedef struct _GDateParseTokens GDateParseTokens;

#define NUM_LEN 10

static void
g_date_fill_parse_tokens (const gchar *str, GDateParseTokens *pt)
{
  gchar num[4][NUM_LEN+1];
  gint i;
  const guchar *s;
  
  /* We count 4, but store 3; so we can give an error
   * if there are 4.
   */
  num[0][0] = num[1][0] = num[2][0] = num[3][0] = '\0';
  
  s = (const guchar *) str;
  pt->num_ints = 0;
  while (*s && pt->num_ints < 4) 
    {
      
      i = 0;
      while (*s && g_ascii_isdigit (*s) && i < NUM_LEN)
        {
          num[pt->num_ints][i] = *s;
          ++s; 
          ++i;
        }
      
      if (i > 0) 
        {
          num[pt->num_ints][i] = '\0';
          ++(pt->num_ints);
        }
      
      if (*s == '\0') break;
      
      ++s;
    }
  
  pt->n[0] = pt->num_ints > 0 ? atoi (num[0]) : 0;
  pt->n[1] = pt->num_ints > 1 ? atoi (num[1]) : 0;
  pt->n[2] = pt->num_ints > 2 ? atoi (num[2]) : 0;

}

static void g_date_determine_dmy(void)
{
GDate d;
gchar buf[128];
GDateParseTokens testpt;
gint i;

  g_date_clear (&d, 1);              /* clear for scratch use */
  
  
      /* had to pick a random day - don't change this, some strftimes
       * are broken on some days, and this one is good so far. */
      g_date_set_dmy (&d, 4, 7, 1976);
      
      g_date_strftime (buf, 127, "%x", &d);
      
      g_date_fill_parse_tokens (buf, &testpt);
      
      i = 0;
      while (i < testpt.num_ints)
        {
          switch (testpt.n[i])
            {
            case 7:
              dmy_order[i] = G_DATE_MONTH;
              break;
            case 4:
              dmy_order[i] = G_DATE_DAY;
              break;
            //case 76:
              //using_twodigit_years = TRUE; /* FALL THRU */
            case 1976:
              dmy_order[2] = G_DATE_YEAR;
              break;
            }
          ++i;
        }

}        

static void
gtk_date_entry_set_property (GObject         *object,
                             guint            prop_id,
                             const GValue    *value,
                             GParamSpec      *pspec)
{
  GtkDateEntry *date_entry = GTK_DATE_ENTRY (object);

  switch (prop_id)
    {
    case PROP_DATE:
      gtk_date_entry_set_date (date_entry, g_value_get_boxed (value));
      break;

    case PROP_MINDATE:
      gtk_date_entry_set_mindate (date_entry, g_value_get_boxed (value));
      break;

    case PROP_MAXDATE:
      gtk_date_entry_set_maxdate (date_entry, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_date_entry_get_property (GObject         *object,
                             guint            prop_id,
                             GValue          *value,
                             GParamSpec      *pspec)
{
  GtkDateEntry *date_entry = GTK_DATE_ENTRY (object);

  switch (prop_id)
    {
    case PROP_DATE:
      g_value_set_boxed (value, gtk_date_entry_get_date (date_entry));
      break;

    case PROP_MINDATE:
      g_value_set_boxed (value, gtk_date_entry_get_mindate (date_entry));
      break;

    case PROP_MAXDATE:
      g_value_set_boxed (value, gtk_date_entry_get_maxdate (date_entry));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_date_entry_finalize (GObject *object)
{
  GtkDateEntry *date_entry = GTK_DATE_ENTRY (object);

  g_date_free (date_entry->priv->date);
  g_date_free (date_entry->priv->mindate);
  g_date_free (date_entry->priv->maxdate);

  G_OBJECT_CLASS (gtk_date_entry_parent_class)->finalize (object);
}

static void
gtk_date_entry_class_init (GtkDateEntryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gtk_date_entry_set_property;
  gobject_class->get_property = gtk_date_entry_get_property;
  gobject_class->finalize = gtk_date_entry_finalize;

  g_object_class_install_property (gobject_class,
                                   PROP_DATE,
                                   g_param_spec_boxed ("date",
                                                        "Date",
                                                        "The Date",
                                                        G_TYPE_DATE,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_MINDATE,
                                   g_param_spec_boxed ("min-date",
                                                        "Minimum Date",
                                                        "The Minimum Date",
                                                        G_TYPE_DATE,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_MAXDATE,
                                   g_param_spec_boxed ("max-date",
                                                        "Maximum Date",
                                                        "The Maximum Date",
                                                        G_TYPE_DATE,
                                                        G_PARAM_READWRITE));

  dateentry_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkDateEntryClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_date_determine_dmy ();
  g_type_class_add_private (klass, sizeof (GtkDateEntryPrivate));
}

static void
gtk_date_entry_position_popup (GtkDateEntry *date_entry)
{
  gint x, y;
  GtkRequisition req;
  GtkAllocation allocation;
  GtkDateEntryPrivate *priv = date_entry->priv;

  gtk_widget_get_preferred_size (priv->popwin, &req, NULL);
  gdk_window_get_origin (gtk_widget_get_window (priv->button), &x, &y);

  gtk_widget_get_allocation (priv->button, &allocation);

  x += allocation.x + allocation.width - req.width;
  y += allocation.y + allocation.height;

  if (x < 0)
    x = 0;

  if (y < 0)
    y = 0;

  gtk_window_move (GTK_WINDOW (priv->popwin), x, y);
}

static void
gtk_date_entry_popup_display (GtkDateEntry *date_entry)
{
  GtkDateEntryPrivate *priv = date_entry->priv;

  g_date_set_parse (priv->date, gtk_entry_get_text (GTK_ENTRY (priv->entry)));
  if (g_date_valid (priv->date))
    {
      g_signal_handler_block (priv->calendar, priv->day_selected_signal);
      gtk_calendar_select_month (GTK_CALENDAR (priv->calendar),
                                 g_date_get_month (priv->date) - 1,
                                 g_date_get_year (priv->date));
      gtk_calendar_select_day (GTK_CALENDAR (priv->calendar),
                               g_date_get_day (priv->date));
      g_signal_handler_unblock (priv->calendar, priv->day_selected_signal);
    }

  gtk_date_entry_position_popup (date_entry);
  gtk_widget_show (priv->popwin);
  gtk_grab_add (priv->popwin);

  gdk_device_grab (gdk_device_manager_get_client_pointer (gdk_display_get_device_manager (gdk_display_get_default ())),
                   gtk_widget_get_window (priv->popwin),
                   GDK_OWNERSHIP_APPLICATION,
                   TRUE,
                   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
                   NULL,
                   GDK_CURRENT_TIME);
}

static void
gtk_date_entry_set_text (GtkDateEntry *date_entry)
{
  gchar buffer[50];
  GtkDateEntryPrivate *priv = date_entry->priv;

  g_date_strftime (buffer, sizeof (buffer), "%x", priv->date);
  gtk_entry_set_text (GTK_ENTRY (priv->entry), buffer);
}

static void
gtk_date_entry_tokens (GtkDateEntry *date_entry)
{
  GtkDateEntryPrivate *priv = date_entry->priv;
  GDateParseTokens pt;

  g_date_fill_parse_tokens (gtk_entry_get_text (GTK_ENTRY (priv->entry)), &pt);

  /* initialize with today's date */
  g_date_set_time_t (priv->date, time (NULL));

  switch (pt.num_ints)
    {
      case 1:
        if (g_date_valid_day (pt.n[0]))
          g_date_set_day (priv->date, pt.n[0]);
          break;

      case 2:
        if (dmy_order[0] != G_DATE_YEAR)
          {
            if (dmy_order[0] == G_DATE_DAY)
              {
                if (g_date_valid_day (pt.n[0]))
                  g_date_set_day (priv->date, pt.n[0]);
                if (g_date_valid_month (pt.n[1]))
                  g_date_set_month (priv->date, pt.n[1]);
              }
            else
              {
                if (g_date_valid_day (pt.n[1]))
                  g_date_set_day (priv->date, pt.n[1]);
                if (g_date_valid_month (pt.n[0]))
                  g_date_set_month (priv->date, pt.n[0]);
              }
           }
         break;
    }
}

static void
gtk_date_entry_parse_text (GtkDateEntry *date_entry)
{
  GtkDateEntryPrivate *priv = date_entry->priv;

  /* 1) we parse the string according to the locale */
  g_date_set_parse (priv->date, gtk_entry_get_text (GTK_ENTRY (priv->entry)));

  /* 2) give a try to tokens: day, day/month, month/day */
  if (!g_date_valid (priv->date))
    gtk_date_entry_tokens (date_entry);

  /* 3) if date is still invalid, consider today */
  if (!g_date_valid (priv->date))
    g_date_set_time_t (priv->date, time (NULL));

  gtk_date_entry_set_text (date_entry);
}

static gboolean
gtk_date_entry_entry_focus_out (GtkWidget *widget, GdkEvent *event, GtkDateEntry *date_entry)
{
  gtk_date_entry_parse_text (date_entry);
  return FALSE;
}

static void
gtk_date_entry_button_toggled (GtkToggleButton *button, GtkDateEntry *date_entry)
{
  if (gtk_toggle_button_get_active (button))
    {
      gtk_date_entry_parse_text (date_entry);
      gtk_date_entry_popup_display (date_entry);
    }
  else
    {
      gtk_widget_hide (date_entry->priv->popwin);
      gtk_grab_remove (date_entry->priv->popwin);
      gdk_device_ungrab (gdk_device_manager_get_client_pointer (gdk_display_get_device_manager (gdk_display_get_default ())),
                         GDK_CURRENT_TIME);
    }
}

static void
gtk_date_entry_calendar_year_changed (GtkCalendar *calendar, GtkDateEntry *date_entry)
{
  guint calendar_year, date_year;

  gtk_calendar_get_date (calendar, &calendar_year, NULL, NULL);

  date_year = g_date_get_year (date_entry->priv->mindate);
  if (calendar_year < date_year)
    g_object_set (calendar, "year", date_year, NULL);
  else
    {
      date_year = g_date_get_year (date_entry->priv->maxdate);
      if (calendar_year > date_year)
        g_object_set (calendar, "year", date_year, NULL);
    }
}

static void
gtk_date_entry_calendar_day_selected (GtkCalendar *calendar, GtkDateEntry *date_entry)
{
  guint year, month, day;
  GDate date;

  gtk_calendar_get_date (calendar, &year, &month, &day);
  g_date_clear (&date, 1);
  g_date_set_dmy (&date, day, month + 1, year);
  gtk_date_entry_set_date (date_entry, &date);
}

static void
gtk_date_entry_calendar_day_selected_double (GtkCalendar *calendar, GtkDateEntry *date_entry)
{
  gtk_date_entry_calendar_day_selected (calendar, date_entry);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (date_entry->priv->button), FALSE);
}

static GtkWidget *
gtk_date_entry_create_button (void)
{
  GtkWidget *button;
  GtkStyleContext *context;
  GtkCssProvider *css;
  const gchar * button_style =
	"* {\n"
	  "-GtkButton-default-border : 0;\n"
	  "-GtkButton-default-outside-border : 0;\n"
	  "-GtkButton-inner-border: 0;\n"
	  "-GtkWidget-focus-line-width : 0;\n"
	  "-GtkWidget-focus-padding : 0;\n"
	  "padding: 0;\n"
	"}";

  css = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (css, button_style, -1, NULL);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "image", gtk_image_new_from_icon_name ("x-office-calendar", GTK_ICON_SIZE_BUTTON),
                         "visible", TRUE,
                         "focus-on-click", FALSE,
                         NULL);
  context = gtk_widget_get_style_context (button);
  gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  return button;
}

static gboolean
gtk_date_entry_popwin_button_press (GtkWidget *widget, GdkEvent *event, GtkDateEntry *date_entry)
{
  GtkWidget *child = gtk_get_event_widget (event);

  if (child != widget)
    while (child)
     {
       child = gtk_widget_get_parent (child);
       if (child == widget)
         return FALSE;
     }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (date_entry->priv->button), FALSE);
  return TRUE;
}

static gboolean
gtk_date_entry_popwin_key_press (GtkWidget *widget, GdkEventKey *event, GtkDateEntry *date_entry)
{
  if (event->keyval != GDK_KEY_Escape)
    return FALSE;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (date_entry->priv->button), FALSE);
  return TRUE;
}

static void
gtk_date_entry_init (GtkDateEntry *date_entry)
{
  GtkDateEntryPrivate *priv;
  GtkWidget *frame;

  date_entry->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE (date_entry, GTK_TYPE_DATE_ENTRY, GtkDateEntryPrivate);

  priv->date = g_date_new ();
  g_date_set_time_t (priv->date, time (NULL));

  priv->mindate = g_date_new ();
  g_date_set_dmy (priv->mindate, 1, 1, 1);

  priv->maxdate = g_date_new ();
  g_date_set_dmy (priv->maxdate, 31, 12, 3000);

  /* Entry */
  priv->entry = g_object_new (GTK_TYPE_ENTRY,
                              "visible", TRUE,
                              "max-length", 10,
                              "width-chars", 10,
                              NULL);
  gtk_container_add (GTK_CONTAINER (date_entry), priv->entry);
  gtk_date_entry_set_text (date_entry);
  g_signal_connect (priv->entry, "focus-out-event", G_CALLBACK (gtk_date_entry_entry_focus_out), date_entry);

  /* Calendar button */
  priv->button = gtk_date_entry_create_button ();
  gtk_container_add (GTK_CONTAINER (date_entry), priv->button);
  g_signal_connect (priv->button, "toggled", G_CALLBACK (gtk_date_entry_button_toggled), date_entry);

  /* popup window */
  priv->popwin = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_add_events (priv->popwin, GDK_KEY_PRESS_MASK);
  g_signal_connect (priv->popwin, "button-press-event", G_CALLBACK (gtk_date_entry_popwin_button_press), date_entry);
  g_signal_connect (priv->popwin, "key-press-event", G_CALLBACK (gtk_date_entry_popwin_key_press), date_entry);

  frame = g_object_new (GTK_TYPE_FRAME,
                        "shadow-type", GTK_SHADOW_OUT,
                        "visible", TRUE,
                         NULL);
  gtk_container_add (GTK_CONTAINER (priv->popwin), frame);

  /* Calendar */
  priv->calendar = g_object_new (GTK_TYPE_CALENDAR,
                                 "visible", TRUE,
                                 NULL);
  gtk_container_add (GTK_CONTAINER (frame), priv->calendar);
  g_signal_connect (priv->calendar, "prev-year", G_CALLBACK (gtk_date_entry_calendar_year_changed), date_entry);
  g_signal_connect (priv->calendar, "next-year", G_CALLBACK (gtk_date_entry_calendar_year_changed), date_entry);
  g_signal_connect (priv->calendar, "prev-month", G_CALLBACK (gtk_date_entry_calendar_year_changed), date_entry);
  g_signal_connect (priv->calendar, "next-month", G_CALLBACK (gtk_date_entry_calendar_year_changed), date_entry);
  priv->day_selected_signal = g_signal_connect (priv->calendar,
                                                "day-selected",
                                                G_CALLBACK (gtk_date_entry_calendar_day_selected),
                                                date_entry);
  g_signal_connect (priv->calendar,
                    "day-selected-double-click",
                    G_CALLBACK (gtk_date_entry_calendar_day_selected_double),
                    date_entry);
}

GtkWidget *
gtk_date_entry_new ()
{
  return GTK_WIDGET (g_object_new (GTK_TYPE_DATE_ENTRY, NULL));
}

void
gtk_date_entry_set_date (GtkDateEntry *date_entry, const GDate *date)
{
  g_return_if_fail (GTK_IS_DATE_ENTRY (date_entry));
  g_return_if_fail (g_date_valid (date));

  if (g_date_compare (date, date_entry->priv->date) == 0)
    return;

  memcpy (date_entry->priv->date, date, sizeof (GDate));
  g_date_clamp (date_entry->priv->date, date_entry->priv->mindate, date_entry->priv->maxdate);
  gtk_date_entry_set_text (date_entry);
  g_object_notify (G_OBJECT (date_entry), "date");
}

void
gtk_date_entry_set_mindate (GtkDateEntry *date_entry, const GDate *mindate)
{
  g_return_if_fail (GTK_IS_DATE_ENTRY (date_entry));
  g_return_if_fail (g_date_valid (mindate));

  if (g_date_compare (mindate, date_entry->priv->mindate) == 0)
    return;

  memcpy (date_entry->priv->mindate, mindate, sizeof (GDate));
  g_date_clamp (date_entry->priv->date, date_entry->priv->mindate, date_entry->priv->maxdate);
  gtk_date_entry_set_text (date_entry);
  g_object_notify (G_OBJECT (date_entry), "min-date");
}

const GDate *
gtk_date_entry_get_mindate (GtkDateEntry *date_entry)
{
  g_return_val_if_fail (GTK_IS_DATE_ENTRY (date_entry), NULL);

  return date_entry->priv->mindate;
}

void
gtk_date_entry_set_maxdate (GtkDateEntry *date_entry, const GDate *maxdate)
{
  g_return_if_fail (GTK_IS_DATE_ENTRY (date_entry));
  g_return_if_fail (g_date_valid (maxdate));

  if (g_date_compare (maxdate, date_entry->priv->maxdate) == 0)
    return;

  memcpy (date_entry->priv->maxdate, maxdate, sizeof (GDate));
  g_date_clamp (date_entry->priv->date, date_entry->priv->mindate, date_entry->priv->maxdate);
  gtk_date_entry_set_text (date_entry);
  g_object_notify (G_OBJECT (date_entry), "max-date");
}

const GDate *
gtk_date_entry_get_maxdate (GtkDateEntry *date_entry)
{
  g_return_val_if_fail (GTK_IS_DATE_ENTRY (date_entry), NULL);

  return date_entry->priv->maxdate;
}

const GDate *
gtk_date_entry_get_date (GtkDateEntry *date_entry)
{
  g_return_val_if_fail (GTK_IS_DATE_ENTRY (date_entry), NULL);

  return date_entry->priv->date;
}

