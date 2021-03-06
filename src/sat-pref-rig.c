/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
    Gpredict: Real-time satellite tracking and orbit prediction program

    Copyright (C)  2001-2010  Alexandru Csete, OZ9AEC.

    Authors: Alexandru Csete <oz9aec@gmail.com>

    Comments, questions and bugreports should be submitted via
    http://sourceforge.net/projects/gpredict/
    More details can be found at the project home page:

            http://gpredict.oz9aec.net/
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, visit http://www.fsf.org/
*/
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#ifdef HAVE_CONFIG_H
#  include <build-config.h>
#endif
#include "gpredict-utils.h"
#include "sat-cfg.h"
#include "sat-log.h"
#include "compat.h"
#include "radio-conf.h"
#include "sat-pref-rig-data.h"
#include "sat-pref-rig-editor.h"
#include "sat-pref-rig.h"


extern GtkWidget *window; /* dialog window defined in sat-pref.c */


/* private function declarations */
static void          create_rig_list       (void);
static GtkTreeModel *create_and_fill_model (void);
static GtkWidget    *create_buttons        (void);

static void add_cb    (GtkWidget *button, gpointer data);
static void edit_cb   (GtkWidget *button, gpointer data);
static void delete_cb (GtkWidget *button, gpointer data);
static void row_activated_cb (GtkTreeView       *tree_view,
                              GtkTreePath       *path,
                              GtkTreeViewColumn *column,
                              gpointer           user_data);

static void render_name (GtkTreeViewColumn *col,
                         GtkCellRenderer   *renderer,
                         GtkTreeModel      *model,
                         GtkTreeIter       *iter,
                         gpointer           column);
static void render_type (GtkTreeViewColumn *col,
                         GtkCellRenderer   *renderer,
                         GtkTreeModel      *model,
                         GtkTreeIter       *iter,
                         gpointer           column);
static void render_ptt (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column);
static void render_lo  (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column);
static void render_vfo (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column);


/* global objects */
static GtkWidget *addbutton;
static GtkWidget *editbutton;
static GtkWidget *delbutton;
static GtkWidget *riglist;


/** \brief Create and initialise widgets for the radios tab. */
GtkWidget *sat_pref_rig_create ()
{
    GtkWidget *vbox;    /* vbox containing the list part and the details part */
    GtkWidget *swin;


    vbox = gtk_vbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);

    /* create qth list and pack into scrolled window */
    create_rig_list ();
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (swin), riglist);

    gtk_box_pack_start (GTK_BOX (vbox), swin, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), create_buttons (), FALSE, FALSE, 0);

    return vbox;
}


/** \brief Create Radio configuration list widget.
 *
 */
static void create_rig_list ()
{
    GtkTreeModel      *model;
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;


    riglist = gtk_tree_view_new ();
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (riglist), TRUE);

    model = create_and_fill_model ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (riglist), model);
    g_object_unref (model);

    /* Conf name */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Config Name"), renderer,
                                                        "text", RIG_LIST_COL_NAME,
                                                        NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_name,
                                             GUINT_TO_POINTER(RIG_LIST_COL_NAME),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);

    /* Host */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Host"), renderer,
                                                       "text", RIG_LIST_COL_HOST,
                                                        NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);
    
    /* port */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Port"), renderer,
                                                       "text", RIG_LIST_COL_PORT,
                                                        NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);

    /* rig type */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Rig Type"), renderer,
                                                       "text", RIG_LIST_COL_TYPE,
                                                        NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_type,
                                             GUINT_TO_POINTER(RIG_LIST_COL_TYPE),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);

    /* PTT */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("PTT Status"), renderer,
                                                       "text", RIG_LIST_COL_PTT,
                                                        NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_ptt,
                                             GUINT_TO_POINTER(RIG_LIST_COL_PTT),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);
    
    /* VFO Up */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("VFO Up"), renderer,
                                                       "text", RIG_LIST_COL_VFOUP,
                                                        NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_vfo,
                                             GUINT_TO_POINTER(RIG_LIST_COL_VFOUP),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);
    
    /* VFO Down */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("VFO Down"), renderer,
                                                       "text", RIG_LIST_COL_VFODOWN,
                                                        NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_vfo,
                                             GUINT_TO_POINTER(RIG_LIST_COL_VFODOWN),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);
    
    /* transverter down */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("LO Down"), renderer,
                                                       "text", RIG_LIST_COL_LO,
                                                       NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_lo,
                                             GUINT_TO_POINTER(RIG_LIST_COL_LO),
                                             NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);
    
    /* transverter up */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("LO Up"), renderer,
                                                    "text", RIG_LIST_COL_LOUP,
                                                    NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             render_lo,
                                             GUINT_TO_POINTER(RIG_LIST_COL_LOUP),
                                                     NULL);
    gtk_tree_view_insert_column (GTK_TREE_VIEW (riglist), column, -1);

    g_signal_connect (riglist, "row-activated", G_CALLBACK (row_activated_cb), riglist);
}


/** \brief Create data storage for radio configuration list. */
static GtkTreeModel *create_and_fill_model ()
{
    GtkListStore *liststore;    /* the list store data structure */
    GtkTreeIter  item;      /* new item added to the list store */
    GDir         *dir = NULL;   /* directory handle */
    GError       *error = NULL; /* error flag and info */
    gchar        *dirname;      /* directory name */
    gchar       **vbuff;
    const gchar  *filename;     /* file name */
    radio_conf_t  conf;

    /* create a new list store */
    liststore = gtk_list_store_new (RIG_LIST_COL_NUM,
                                    G_TYPE_STRING,    // name
                                    G_TYPE_STRING,    // host
                                    G_TYPE_INT,       // port
                                    G_TYPE_INT,       // type
                                    G_TYPE_INT,       // PTT
                                    G_TYPE_INT,       // VFO Up
                                    G_TYPE_INT,       // VFO Down
                                    G_TYPE_DOUBLE,    // LO DOWN
                                    G_TYPE_DOUBLE     // LO UO
                                   );

     gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE(liststore),RIG_LIST_COL_NAME,GTK_SORT_ASCENDING);
    /* open configuration directory */
    dirname = get_hwconf_dir ();
    
    dir = g_dir_open (dirname, 0, &error);
    if (dir) {
        /* read each .rig file */
        while ((filename = g_dir_read_name (dir))) {
            
            if (g_str_has_suffix (filename, ".rig")) {
                
                vbuff = g_strsplit (filename, ".rig", 0);
                conf.name = g_strdup (vbuff[0]);
                g_strfreev (vbuff);
                if (radio_conf_read (&conf)) {
                    /* insert conf into liststore */
                    gtk_list_store_append (liststore, &item);
                    gtk_list_store_set (liststore, &item,
                                        RIG_LIST_COL_NAME, conf.name,
                                        RIG_LIST_COL_HOST, conf.host,
                                        RIG_LIST_COL_PORT, conf.port,
                                        RIG_LIST_COL_TYPE, conf.type,
                                        RIG_LIST_COL_PTT, conf.ptt,
                                        RIG_LIST_COL_VFOUP, conf.vfoUp,
                                        RIG_LIST_COL_VFODOWN, conf.vfoDown,
                                        RIG_LIST_COL_LO, conf.lo,
                                        RIG_LIST_COL_LOUP, conf.loup,
                                        -1);
                    
                    sat_log_log (SAT_LOG_LEVEL_DEBUG,
                                 _("%s:%d: Read %s"),
                                 __FILE__, __LINE__, filename);
                    /* clean up memory */
                    if (conf.name)
                        g_free (conf.name);
                    
                    if (conf.host)
                        g_free (conf.host);
                    
                }
                else {
                    /* there was an error */
                    sat_log_log (SAT_LOG_LEVEL_ERROR,
                                 _("%s:%d: Failed to read %s"),
                                 __FILE__, __LINE__, conf.name);
                    
                    g_free (conf.name);
                }
            }
        }
    }
    else {
        sat_log_log (SAT_LOG_LEVEL_ERROR,
                     _("%s:%d: Failed to open hwconf dir (%s)"),
                       __FILE__, __LINE__, error->message);
        g_clear_error (&error);
    }

    g_free (dirname);
    g_dir_close (dir);

    return GTK_TREE_MODEL (liststore);
}


/** \brief Create buttons.
 *  \param riglist The GtkTreeView widget containing the rig data.
 *  \return A button box containing the buttons.
 *
 * This function creates and initialises the three buttons below the rig list.
 * The treeview widget is needed by the buttons when they are activated.
 *
 */
static GtkWidget *create_buttons (void)
{
    GtkWidget *box;

    /* add button */
    addbutton = gpredict_hstock_button (GTK_STOCK_ADD,
                                        _("Add New"),
                                        _("Add a new radio to the list"));
    g_signal_connect (addbutton, "clicked", G_CALLBACK (add_cb), NULL);

    /* edit button */
    editbutton = gpredict_hstock_button (GTK_STOCK_EDIT,
                                         _("Edit"),
                                         _("Edit the currently selected radio"));
    g_signal_connect (editbutton, "clicked", G_CALLBACK (edit_cb), NULL);

    /* delete button; don't forget to delete file.... */
    delbutton = gpredict_hstock_button (GTK_STOCK_DELETE,
                                        _("Delete"),
                                        _("Delete the selected radio"));
    g_signal_connect (delbutton, "clicked", G_CALLBACK (delete_cb), NULL);

    /* vertical button box */
    box = gtk_hbutton_box_new ();
    gtk_button_box_set_layout (GTK_BUTTON_BOX (box), GTK_BUTTONBOX_START);

    gtk_container_add (GTK_CONTAINER (box), addbutton);
    gtk_container_add (GTK_CONTAINER (box), editbutton);
    gtk_container_add (GTK_CONTAINER (box), delbutton);


    return box;
}



/** \brief User pressed cancel. Any changes to config must be cancelled.
 */
void sat_pref_rig_cancel ()
{
}


/** \brief User pressed OK. Any changes should be stored in config.
 * 
 * First, all .grc files are deleted, whereafter the radio configurations in
 * the riglist are saved one by one.
 */
void sat_pref_rig_ok     ()
{
    GDir         *dir = NULL;   /* directory handle */
    GError       *error = NULL; /* error flag and info */
    gchar        *buff,*dirname;
    const gchar  *filename;
    GtkTreeIter   iter;      /* new item added to the list store */
    GtkTreeModel *model;
    guint         i,n;
    
    radio_conf_t conf = {
        .name  = NULL,
        .host  = NULL,
        .port  = 4532,
        .type  = RIG_TYPE_RX,
        .ptt   = 0,
        .vfoUp = 0,
        .vfoDown = 0,
        .lo    = 0.0,
        .loup  = 0.0,
    };

    
    /* delete all .rig files */
    dirname = get_hwconf_dir ();
    
    dir = g_dir_open (dirname, 0, &error);
    if (dir) {
        /* read each .rig file */
        while ((filename = g_dir_read_name (dir))) {

            if (g_str_has_suffix (filename, ".rig")) {

                buff = g_strconcat (dirname, G_DIR_SEPARATOR_S, filename, NULL);
                g_remove (buff);
                g_free (buff);
            }
        }
    }

    g_free (dirname);
    g_dir_close (dir);
    
    /* create new .rig files for the radios in the riglist */
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (riglist));
    n = gtk_tree_model_iter_n_children (model, NULL);
    for (i = 0; i < n; i++) {
        
        /* get radio conf */
        if (gtk_tree_model_iter_nth_child (model, &iter, NULL, i)) {
        
            /* store conf */
            gtk_tree_model_get (model, &iter,
                                RIG_LIST_COL_NAME, &conf.name,
                                RIG_LIST_COL_HOST, &conf.host,
                                RIG_LIST_COL_PORT, &conf.port,
                                RIG_LIST_COL_TYPE, &conf.type,
                                RIG_LIST_COL_PTT, &conf.ptt,
                                RIG_LIST_COL_VFOUP, &conf.vfoUp,
                                RIG_LIST_COL_VFODOWN, &conf.vfoDown,
                                RIG_LIST_COL_LO, &conf.lo,
                                RIG_LIST_COL_LOUP, &conf.loup,
                                -1);
            radio_conf_save (&conf);
        
            /* free conf buffer */
            if (conf.name)
                g_free (conf.name);
            
            if (conf.host)
                g_free (conf.host);
            
        }
        else {
            sat_log_log (SAT_LOG_LEVEL_ERROR,
                         _("%s: Failed to get RIG %s"),
                           __FUNCTION__, i);
        }
    }
    
}


/** \brief Add a new radio configuration
 * \param button Pointer to the Add button.
 * \param data User data (null).
 * 
 * This function executes the radio configuration editor with the "new"
 * flag set to TRUE.
 */
static void add_cb    (GtkWidget *button, gpointer data)
{
    GtkTreeIter  item;      /* new item added to the list store */
    GtkListStore *liststore;
    
    (void) button; /* avoid unused parameter compiler warning */
    (void) data; /* avoid unused parameter compiler warning */
        
    radio_conf_t conf = {
        .name  = NULL,
        .host  = NULL,
        .port  = 4532,
        .type  = RIG_TYPE_RX,
        .ptt   = 0,
        .vfoUp = 0,
        .vfoDown = 0,
        .lo    = 0.0,
        .loup  = 0.0,
    };
    
    /* run rig conf editor */
    sat_pref_rig_editor_run (&conf);
    
    /* add new rig to the list */
    if (conf.name != NULL) {
        liststore = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (riglist)));
        gtk_list_store_append (liststore, &item);
        gtk_list_store_set (liststore, &item,
                            RIG_LIST_COL_NAME, conf.name,
                            RIG_LIST_COL_HOST, conf.host,
                            RIG_LIST_COL_PORT, conf.port,
                            RIG_LIST_COL_TYPE, conf.type,
                            RIG_LIST_COL_PTT, conf.ptt,
                            RIG_LIST_COL_VFOUP, conf.vfoUp,
                            RIG_LIST_COL_VFODOWN, conf.vfoDown,
                            RIG_LIST_COL_LO, conf.lo,
                            RIG_LIST_COL_LOUP, conf.loup,
                            -1);
        
        g_free (conf.name);
        
        if (conf.host != NULL)
            g_free (conf.host);
        
    }
}


/** \brief Add a new radio configuration
 * \param button Pointer to the Add button.
 * \param data User data (null).
 * 
 * This function executes the radio configuration editor with the "new"
 * flag set to TRUE.
 */
static void edit_cb   (GtkWidget *button, gpointer data)
{
    GtkTreeModel     *model = gtk_tree_view_get_model (GTK_TREE_VIEW(riglist));
    GtkTreeModel     *selmod;
    GtkTreeSelection *selection;
    GtkTreeIter       iter;
    
    (void) button; /* avoid unused parameter compiler warning */
    (void) data; /* avoid unused parameter compiler warning */

    radio_conf_t conf = {
        .name  = NULL,
        .host  = NULL,
        .port  = 4532,
        .type  = RIG_TYPE_RX,
        .ptt   = 0,
        .vfoUp = 0,
        .vfoDown = 0,
        .lo    = 0.0,
        .loup  = 0.0,
    };

    
    /* If there are no entries, we have a bug since the button should 
    have been disabled. */
    if (gtk_tree_model_iter_n_children (model, NULL) < 1) {

        sat_log_log (SAT_LOG_LEVEL_ERROR,
                     _("%s:%s: Edit button should have been disabled."),
                       __FILE__, __FUNCTION__);
        //gtk_widget_set_sensitive (button, FALSE);
        
        return;
    }
    
    /* get selected row
    FIXME: do we really need to work with two models?
    */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (riglist));
    if (gtk_tree_selection_get_selected(selection, &selmod, &iter)) {
        gtk_tree_model_get (model, &iter,
                            RIG_LIST_COL_NAME, &conf.name,
                            RIG_LIST_COL_HOST, &conf.host,
                            RIG_LIST_COL_PORT, &conf.port,
                            RIG_LIST_COL_TYPE, &conf.type,
                            RIG_LIST_COL_PTT, &conf.ptt,
                            RIG_LIST_COL_VFOUP, &conf.vfoUp,
                            RIG_LIST_COL_VFODOWN, &conf.vfoDown,
                            RIG_LIST_COL_LO, &conf.lo,
                            RIG_LIST_COL_LOUP, &conf.loup,
                            -1);

    }
    else {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                         GTK_DIALOG_MODAL |
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                        _("Select the radio you want to edit\n"\
                                          "and try again!"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);

        return;
    }

    /* run radio configuration editor */
    sat_pref_rig_editor_run (&conf);
    
    /* apply changes */
    if (conf.name != NULL) {
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                            RIG_LIST_COL_NAME, conf.name,
                            RIG_LIST_COL_HOST, conf.host,
                            RIG_LIST_COL_PORT, conf.port,
                            RIG_LIST_COL_TYPE, conf.type,
                            RIG_LIST_COL_PTT, conf.ptt,
                            RIG_LIST_COL_VFOUP, conf.vfoUp,
                            RIG_LIST_COL_VFODOWN, conf.vfoDown,
                            RIG_LIST_COL_LO, conf.lo,
                            RIG_LIST_COL_LOUP, conf.loup,
                            -1);
        
    }

    /* clean up memory */
    if (conf.name)
        g_free (conf.name);
        
    if (conf.host != NULL)
        g_free (conf.host);

}


/** \brief Delete selected radio configuration
 *
 * This function is called when the user clicks the Delete button.
 * 
 */
static void delete_cb (GtkWidget *button, gpointer data)
{
    GtkTreeModel     *model = gtk_tree_view_get_model (GTK_TREE_VIEW(riglist));
    GtkTreeSelection *selection;
    GtkTreeIter       iter;

    (void) button; /* avoid unused parameter compiler warning */
    (void) data; /* avoid unused parameter compiler warning */

    /* If there are no entries, we have a bug since the button should 
       have been disabled. */
    if (gtk_tree_model_iter_n_children (model, NULL) < 1) {

        sat_log_log (SAT_LOG_LEVEL_ERROR,
                     _("%s:%s: Delete button should have been disabled."),
                     __FILE__, __FUNCTION__);
        //gtk_widget_set_sensitive (button, FALSE);
        
        return;
    }
    
    /* get selected row */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (riglist));
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        gtk_list_store_remove (GTK_LIST_STORE(model), &iter);
    }
    else {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                         GTK_DIALOG_MODAL |
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         _("Select the radio you want to delete\n"\
                                           "and try again!"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);

    }
}

static void row_activated_cb (GtkTreeView       *tree_view,
                              GtkTreePath       *path,
                              GtkTreeViewColumn *column,
                              gpointer           user_data)
{
    (void) tree_view; /* avoid unused parameter compiler warning */
    (void) path; /* avoid unused parameter compiler warning */
    (void) column; /* avoid unused parameter compiler warning */
    (void) user_data; /* avoid unused parameter compiler warning */

    edit_cb (editbutton, NULL);
}


/** \brief Render configuration name.
 * \param col Pointer to the tree view column.
 * \param renderer Pointer to the renderer.
 * \param model Pointer to the tree model.
 * \param iter Pointer to the tree iterator.
 * \param column The column number in the model.
 * 
 * This function renders the configuration name onto the riglist. Although
 * the configuration name is a plain string, it contains the file name of the
 * configuration file and we want to strip the .rig extension.
*/
static void render_name (GtkTreeViewColumn *col,
                         GtkCellRenderer   *renderer,
                         GtkTreeModel      *model,
                         GtkTreeIter       *iter,
                         gpointer           column)
{
    gchar  *fname;
    gchar **buff;
    guint   coli = GPOINTER_TO_UINT (column);

    (void) col; /* avoid unused parameter compiler warning */
    
    gtk_tree_model_get (model, iter, coli, &fname, -1);

    buff = g_strsplit (fname, ".rig", 0);
    g_object_set (renderer, "text", buff[0], NULL);
    
    g_strfreev (buff);
    g_free (fname);
}


/** \brief Render radio type.
 * \param col Pointer to the tree view column.
 * \param renderer Pointer to the renderer.
 * \param model Pointer to the tree model.
 * \param iter Pointer to the tree iterator.
 * \param column The column number in the model.
 * 
 * This function renders the radio type onto the riglist.
*/
static void render_type (GtkTreeViewColumn *col,
                         GtkCellRenderer   *renderer,
                         GtkTreeModel      *model,
                         GtkTreeIter       *iter,
                         gpointer           column)
{
    guint   type;
    guint   coli = GPOINTER_TO_UINT (column);

    (void) col; /* avoid unused parameter compiler warning */    

    gtk_tree_model_get (model, iter, coli, &type, -1);

    switch (type) {
    case RIG_TYPE_RX:
        g_object_set (renderer, "text", _("RX only"), NULL);
        break;

    case RIG_TYPE_TX:
        g_object_set (renderer, "text", _("TX only"), NULL);
        break;

    case RIG_TYPE_TRX:
        g_object_set (renderer, "text", _("RX + TX"), NULL);
        break;

    case RIG_TYPE_DUPLEX:
        g_object_set (renderer, "text", _("Duplex"), NULL);
        break;

    case RIG_TYPE_TOGGLE_AUTO:
        g_object_set (renderer, "text", _("FT817/857/897 (auto)"), NULL);
        break;

    case RIG_TYPE_TOGGLE_MAN:
        g_object_set (renderer, "text", _("FT817/857/897 (man)"), NULL);
        break;

    default:
        g_object_set (renderer, "text", _("ERROR"), NULL);
        break;
    }
    
}


/** \brief Render PTT status usage.
 * \param col Pointer to the tree view column.
 * \param renderer Pointer to the renderer.
 * \param model Pointer to the tree model.
 * \param iter Pointer to the tree iterator.
 * \param column The column number in the model.
 * 
 * This function renders the PTT status usage onto the riglist.
*/
static void render_ptt (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column)
{
    gint    ptt;
    guint   coli = GPOINTER_TO_UINT (column);
    
    (void) col; /* avoid unused parameter compiler warning */

    gtk_tree_model_get (model, iter, coli, &ptt, -1);

    switch (ptt) {
        case PTT_TYPE_NONE:
            g_object_set (renderer, "text", _("None"), NULL);
            break;
        case PTT_TYPE_CAT:
            g_object_set (renderer, "text", _("PTT"), NULL);
            break;
        case PTT_TYPE_DCD:
            g_object_set (renderer, "text", _("DCD"), NULL);
            break;
        default:
            g_object_set (renderer, "text", _("None"), NULL);
            break;
    }
}


/** \brief Render Local Oscillator frequency.
 * \param col Pointer to the tree view column.
 * \param renderer Pointer to the renderer.
 * \param model Pointer to the tree model.
 * \param iter Pointer to the tree iterator.
 * \param column The column number in the model.
 *
 * This function is used to render the local oscillator frequency. We
 * need a special renderer so that we can automatically render as MHz
 * instead of Hz.
 */
static void render_lo (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column)
{
    gdouble  number;
    gchar   *buff;
    guint    coli = GPOINTER_TO_UINT (column);
    
    (void) col; /* avoid unused parameter compiler warning */

    gtk_tree_model_get (model, iter, coli, &number, -1);

    /* convert to MHz */
    number /= 1000000.0;
    buff = g_strdup_printf ("%.0f MHz", number);
    g_object_set (renderer, "text", buff, NULL);
    g_free (buff);
}

/** \brief Render VFO selection.
 * \param col Pointer to the tree view column.
 * \param renderer Pointer to the renderer.
 * \param model Pointer to the tree model.
 * \param iter Pointer to the tree iterator.
 * \param column The column number in the model.
 *
 * This function is used to render the VFO up/down selections for
 * full duplex radios.
 */
static void render_vfo (GtkTreeViewColumn *col,
                        GtkCellRenderer   *renderer,
                        GtkTreeModel      *model,
                        GtkTreeIter       *iter,
                        gpointer           column)
{
    gint     number;
    gchar   *buff;
    guint    coli = GPOINTER_TO_UINT (column);
    
    (void) col; /* avoid unused parameter compiler warning */

    gtk_tree_model_get (model, iter, coli, &number, -1);

    switch (number) {
        
        case VFO_A:
            buff = g_strdup_printf ("VFO A");
            break;
        
        case VFO_B:
            buff = g_strdup_printf ("VFO B");
            break;
        
        case VFO_MAIN:
            buff = g_strdup_printf ("Main");
            break;
        
        case VFO_SUB:
            buff = g_strdup_printf ("Sub");
            break;
            
        default:
            buff = g_strdup_printf ("-");
            break;
    }

    g_object_set (renderer, "text", buff, NULL);
    g_free (buff);
}

