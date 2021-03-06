﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    public partial class OptionsDialog : Form
    {
        public OptionsDialog()
        {
            InitializeComponent();

            btnOK.DialogResult = DialogResult.OK;
            btnCancel.DialogResult = DialogResult.Cancel;
            DefaultPluginName = Properties.Settings.Default.CurrentPluginFolder;

            textBoxDefaultPlugin.Text = Properties.Settings.Default.CurrentPluginFolder;
        }

        //Properties
        public String DefaultFolderName { get; set; }
        public String DefaultPluginName { get; set; }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                DefaultFolderName = fbd.SelectedPath;
            }
        }

        private void buttonBrowse2_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                DefaultPluginName = fbd.SelectedPath;
                textBoxDefaultPlugin.Text = DefaultPluginName;
            }
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            DefaultPluginName = textBoxDefaultPlugin.Text;
            Properties.Settings.Default.CurrentPluginFolder = DefaultPluginName;
        }
    }
}
