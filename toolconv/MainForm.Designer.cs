namespace ToolConv
{
    partial class MainForm
    {
        /// <summary>
        /// Variable del diseñador necesaria.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Limpiar los recursos que se estén usando.
        /// </summary>
        /// <param name="disposing">true si los recursos administrados se deben desechar; false en caso contrario.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Código generado por el Diseñador de Windows Forms

        /// <summary>
        /// Método necesario para admitir el Diseñador. No se puede modificar
        /// el contenido de este método con el editor de código.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.groupBoxFolders = new System.Windows.Forms.GroupBox();
            this.buttonPickOutput = new System.Windows.Forms.Button();
            this.buttonPickInput = new System.Windows.Forms.Button();
            this.textBoxFolderOutput = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxFolderInput = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.buttonProcess = new System.Windows.Forms.Button();
            this.listViewFiles = new System.Windows.Forms.ListView();
            this.columnHeaderFiles = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemType = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemOpenFile = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemOpenFolder = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRemove = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRemoveSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabControlSettings = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.label23 = new System.Windows.Forms.Label();
            this.checkBoxBNO_reject_spaces = new System.Windows.Forms.CheckBox();
            this.textBoxBNOblacklist = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.textBoxBNOwhitelist = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.checkBoxKDT_opacity_slice = new System.Windows.Forms.CheckBox();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label32 = new System.Windows.Forms.Label();
            this.checkBoxKDT_sub_slice = new System.Windows.Forms.CheckBox();
            this.checkBoxKDT_uper_slice = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textBoxKDT_image_magick_exec = new System.Windows.Forms.TextBox();
            this.label30 = new System.Windows.Forms.Label();
            this.label24 = new System.Windows.Forms.Label();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.label33 = new System.Windows.Forms.Label();
            this.numericUpDownKDT_max_dimmen = new System.Windows.Forms.NumericUpDown();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.numericUpDownKDT_scale_factor = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownKDT_scale_factor_limit = new System.Windows.Forms.NumericUpDown();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.label10 = new System.Windows.Forms.Label();
            this.comboBoxKDT_downscale_procedure = new System.Windows.Forms.ComboBox();
            this.comboBoxKDT_pixel_format = new System.Windows.Forms.ComboBox();
            this.label11 = new System.Windows.Forms.Label();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.checkBoxKDT_force_vq_on_small = new System.Windows.Forms.CheckBox();
            this.checkBoxKDT_force_square_vq = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.checkBoxKDT_lzss = new System.Windows.Forms.CheckBox();
            this.label7 = new System.Windows.Forms.Label();
            this.numericUpDownKDT_quality = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.comboBoxKDT_dither_algorithm = new System.Windows.Forms.ComboBox();
            this.label9 = new System.Windows.Forms.Label();
            this.comboBoxKDT_scale_agorithm = new System.Windows.Forms.ComboBox();
            this.checkBoxKDT_rgb565 = new System.Windows.Forms.CheckBox();
            this.checkBoxKDT_vq = new System.Windows.Forms.CheckBox();
            this.checkBoxKDT_no_twiddled = new System.Windows.Forms.CheckBox();
            this.label6 = new System.Windows.Forms.Label();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.label25 = new System.Windows.Forms.Label();
            this.groupBox8 = new System.Windows.Forms.GroupBox();
            this.checkBoxKDM_mono = new System.Windows.Forms.CheckBox();
            this.numericUpDownKDM_sample_rate = new System.Windows.Forms.NumericUpDown();
            this.label19 = new System.Windows.Forms.Label();
            this.checkBoxKDM_silence = new System.Windows.Forms.CheckBox();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.comboBoxKDM_fps = new System.Windows.Forms.ComboBox();
            this.checkBoxKDM_no_save_original_resolution = new System.Windows.Forms.CheckBox();
            this.numericUpDownKDM_mpeg_bitrate = new System.Windows.Forms.NumericUpDown();
            this.label31 = new System.Windows.Forms.Label();
            this.numericUpDownKDM_two_pass_bitrate = new System.Windows.Forms.NumericUpDown();
            this.label15 = new System.Windows.Forms.Label();
            this.checkBoxKDM_no_progress = new System.Windows.Forms.CheckBox();
            this.checkBoxKDM_hq = new System.Windows.Forms.CheckBox();
            this.label29 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.numericUpDownKDM_cue_interval = new System.Windows.Forms.NumericUpDown();
            this.label16 = new System.Windows.Forms.Label();
            this.comboBoxKDM_dither_algorithm = new System.Windows.Forms.ComboBox();
            this.label17 = new System.Windows.Forms.Label();
            this.comboBoxKDM_scale_algorithm = new System.Windows.Forms.ComboBox();
            this.checkBoxKDM_small_resolution = new System.Windows.Forms.CheckBox();
            this.label14 = new System.Windows.Forms.Label();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.label26 = new System.Windows.Forms.Label();
            this.checkBoxSFX_test_only = new System.Windows.Forms.CheckBox();
            this.checkBoxSFX_copy_if_rejected = new System.Windows.Forms.CheckBox();
            this.checkBoxSFX_force_mono = new System.Windows.Forms.CheckBox();
            this.checkBoxSFX_pcm_u8 = new System.Windows.Forms.CheckBox();
            this.checkBoxSFX_auto_sample_rate = new System.Windows.Forms.CheckBox();
            this.label22 = new System.Windows.Forms.Label();
            this.numericUpDownSFX_sample_rate = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSFX_maximum_duration = new System.Windows.Forms.NumericUpDown();
            this.label21 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.groupBox9 = new System.Windows.Forms.GroupBox();
            this.textBoxPROFILE_filename = new System.Windows.Forms.TextBox();
            this.buttonPROFILES_import = new System.Windows.Forms.Button();
            this.buttonPROFILES_export = new System.Windows.Forms.Button();
            this.label28 = new System.Windows.Forms.Label();
            this.label27 = new System.Windows.Forms.Label();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.label34 = new System.Windows.Forms.Label();
            this.numericUpDownKDM_gop = new System.Windows.Forms.NumericUpDown();
            this.groupBoxFolders.SuspendLayout();
            this.tabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.contextMenuStrip.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabControlSettings.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox6.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_max_dimmen)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_scale_factor)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_scale_factor_limit)).BeginInit();
            this.groupBox5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_quality)).BeginInit();
            this.tabPage5.SuspendLayout();
            this.groupBox8.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_sample_rate)).BeginInit();
            this.groupBox7.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_mpeg_bitrate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_two_pass_bitrate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_cue_interval)).BeginInit();
            this.tabPage6.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSFX_sample_rate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSFX_maximum_duration)).BeginInit();
            this.tabPage7.SuspendLayout();
            this.groupBox9.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_gop)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBoxFolders
            // 
            this.groupBoxFolders.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBoxFolders.Controls.Add(this.buttonPickOutput);
            this.groupBoxFolders.Controls.Add(this.buttonPickInput);
            this.groupBoxFolders.Controls.Add(this.textBoxFolderOutput);
            this.groupBoxFolders.Controls.Add(this.label2);
            this.groupBoxFolders.Controls.Add(this.textBoxFolderInput);
            this.groupBoxFolders.Controls.Add(this.label1);
            this.groupBoxFolders.Location = new System.Drawing.Point(13, 12);
            this.groupBoxFolders.Name = "groupBoxFolders";
            this.groupBoxFolders.Size = new System.Drawing.Size(776, 79);
            this.groupBoxFolders.TabIndex = 0;
            this.groupBoxFolders.TabStop = false;
            this.groupBoxFolders.Text = "Folders";
            // 
            // buttonPickOutput
            // 
            this.buttonPickOutput.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonPickOutput.Location = new System.Drawing.Point(695, 45);
            this.buttonPickOutput.Name = "buttonPickOutput";
            this.buttonPickOutput.Size = new System.Drawing.Size(75, 23);
            this.buttonPickOutput.TabIndex = 4;
            this.buttonPickOutput.Text = "Choose";
            this.buttonPickOutput.UseVisualStyleBackColor = true;
            this.buttonPickOutput.Click += new System.EventHandler(this.buttonPickOutput_Click);
            // 
            // buttonPickInput
            // 
            this.buttonPickInput.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonPickInput.Location = new System.Drawing.Point(695, 16);
            this.buttonPickInput.Name = "buttonPickInput";
            this.buttonPickInput.Size = new System.Drawing.Size(75, 23);
            this.buttonPickInput.TabIndex = 1;
            this.buttonPickInput.Text = "Choose";
            this.buttonPickInput.UseVisualStyleBackColor = true;
            this.buttonPickInput.Click += new System.EventHandler(this.buttonPickInput_Click);
            // 
            // textBoxFolderOutput
            // 
            this.textBoxFolderOutput.AllowDrop = true;
            this.textBoxFolderOutput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxFolderOutput.Location = new System.Drawing.Point(58, 45);
            this.textBoxFolderOutput.Name = "textBoxFolderOutput";
            this.textBoxFolderOutput.ReadOnly = true;
            this.textBoxFolderOutput.Size = new System.Drawing.Size(631, 20);
            this.textBoxFolderOutput.TabIndex = 3;
            this.textBoxFolderOutput.DragDrop += new System.Windows.Forms.DragEventHandler(this.textBoxFolder_DragDrop);
            this.textBoxFolderOutput.DragEnter += new System.Windows.Forms.DragEventHandler(this.textBoxFolder_DragEnter);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(7, 48);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(45, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Output: ";
            // 
            // textBoxFolderInput
            // 
            this.textBoxFolderInput.AllowDrop = true;
            this.textBoxFolderInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxFolderInput.Location = new System.Drawing.Point(58, 19);
            this.textBoxFolderInput.Name = "textBoxFolderInput";
            this.textBoxFolderInput.ReadOnly = true;
            this.textBoxFolderInput.Size = new System.Drawing.Size(631, 20);
            this.textBoxFolderInput.TabIndex = 1;
            this.textBoxFolderInput.DragDrop += new System.Windows.Forms.DragEventHandler(this.textBoxFolder_DragDrop);
            this.textBoxFolderInput.DragEnter += new System.Windows.Forms.DragEventHandler(this.textBoxFolder_DragEnter);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(37, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Input: ";
            // 
            // tabControl
            // 
            this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl.Controls.Add(this.tabPage1);
            this.tabControl.Controls.Add(this.tabPage2);
            this.tabControl.Location = new System.Drawing.Point(12, 97);
            this.tabControl.Multiline = true;
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(776, 413);
            this.tabControl.TabIndex = 1;
            // 
            // tabPage1
            // 
            this.tabPage1.AllowDrop = true;
            this.tabPage1.Controls.Add(this.buttonProcess);
            this.tabPage1.Controls.Add(this.listViewFiles);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(768, 387);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Files";
            this.tabPage1.UseVisualStyleBackColor = true;
            this.tabPage1.DragDrop += new System.Windows.Forms.DragEventHandler(this.tabPage1_DragDrop);
            this.tabPage1.DragEnter += new System.Windows.Forms.DragEventHandler(this.tabPage1_DragEnter);
            // 
            // buttonProcess
            // 
            this.buttonProcess.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonProcess.Location = new System.Drawing.Point(612, 358);
            this.buttonProcess.Name = "buttonProcess";
            this.buttonProcess.Size = new System.Drawing.Size(150, 23);
            this.buttonProcess.TabIndex = 1;
            this.buttonProcess.Text = "Process files";
            this.buttonProcess.UseVisualStyleBackColor = true;
            this.buttonProcess.Click += new System.EventHandler(this.buttonProcess_Click);
            // 
            // listViewFiles
            // 
            this.listViewFiles.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewFiles.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderFiles});
            this.listViewFiles.ContextMenuStrip = this.contextMenuStrip;
            this.listViewFiles.FullRowSelect = true;
            this.listViewFiles.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.listViewFiles.HideSelection = false;
            this.listViewFiles.LabelWrap = false;
            this.listViewFiles.Location = new System.Drawing.Point(3, 6);
            this.listViewFiles.Name = "listViewFiles";
            this.listViewFiles.ShowGroups = false;
            this.listViewFiles.Size = new System.Drawing.Size(762, 346);
            this.listViewFiles.TabIndex = 0;
            this.listViewFiles.UseCompatibleStateImageBehavior = false;
            this.listViewFiles.View = System.Windows.Forms.View.Details;
            this.listViewFiles.DoubleClick += new System.EventHandler(this.listViewFiles_DoubleClick);
            this.listViewFiles.KeyUp += new System.Windows.Forms.KeyEventHandler(this.listViewFiles_KeyUp);
            // 
            // columnHeaderFiles
            // 
            this.columnHeaderFiles.Text = "Files";
            this.columnHeaderFiles.Width = 0;
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemType,
            this.toolStripSeparator,
            this.toolStripMenuItemOpenFile,
            this.toolStripMenuItemOpenFolder,
            this.toolStripMenuItemRemove,
            this.toolStripMenuItemRemoveSelected});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.contextMenuStrip.Size = new System.Drawing.Size(255, 120);
            this.contextMenuStrip.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip_Opening);
            // 
            // toolStripMenuItemType
            // 
            this.toolStripMenuItemType.Enabled = false;
            this.toolStripMenuItemType.Name = "toolStripMenuItemType";
            this.toolStripMenuItemType.Size = new System.Drawing.Size(254, 22);
            this.toolStripMenuItemType.Text = "Type";
            // 
            // toolStripSeparator
            // 
            this.toolStripSeparator.Name = "toolStripSeparator";
            this.toolStripSeparator.Size = new System.Drawing.Size(251, 6);
            // 
            // toolStripMenuItemOpenFile
            // 
            this.toolStripMenuItemOpenFile.Name = "toolStripMenuItemOpenFile";
            this.toolStripMenuItemOpenFile.Size = new System.Drawing.Size(254, 22);
            this.toolStripMenuItemOpenFile.Text = "&Open File";
            this.toolStripMenuItemOpenFile.Click += new System.EventHandler(this.toolStripMenuItemOpenFile_Click);
            // 
            // toolStripMenuItemOpenFolder
            // 
            this.toolStripMenuItemOpenFolder.Name = "toolStripMenuItemOpenFolder";
            this.toolStripMenuItemOpenFolder.Size = new System.Drawing.Size(254, 22);
            this.toolStripMenuItemOpenFolder.Text = "Open Containing &Folder";
            this.toolStripMenuItemOpenFolder.Click += new System.EventHandler(this.toolStripMenuItemOpenFolder_Click);
            // 
            // toolStripMenuItemRemove
            // 
            this.toolStripMenuItemRemove.Name = "toolStripMenuItemRemove";
            this.toolStripMenuItemRemove.Size = new System.Drawing.Size(254, 22);
            this.toolStripMenuItemRemove.Text = "&Remove from the list";
            this.toolStripMenuItemRemove.Click += new System.EventHandler(this.toolStripMenuItemRemove_Click);
            // 
            // toolStripMenuItemRemoveSelected
            // 
            this.toolStripMenuItemRemoveSelected.Name = "toolStripMenuItemRemoveSelected";
            this.toolStripMenuItemRemoveSelected.Size = new System.Drawing.Size(254, 22);
            this.toolStripMenuItemRemoveSelected.Text = "Remove selected files from the list";
            this.toolStripMenuItemRemoveSelected.Click += new System.EventHandler(this.toolStripMenuItemMenuRemoveSelected_Click);
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.tabControlSettings);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(768, 387);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Settings";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // tabControlSettings
            // 
            this.tabControlSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControlSettings.Controls.Add(this.tabPage3);
            this.tabControlSettings.Controls.Add(this.tabPage4);
            this.tabControlSettings.Controls.Add(this.tabPage5);
            this.tabControlSettings.Controls.Add(this.tabPage6);
            this.tabControlSettings.Controls.Add(this.tabPage7);
            this.tabControlSettings.Location = new System.Drawing.Point(6, 6);
            this.tabControlSettings.Name = "tabControlSettings";
            this.tabControlSettings.SelectedIndex = 0;
            this.tabControlSettings.Size = new System.Drawing.Size(756, 375);
            this.tabControlSettings.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.label23);
            this.tabPage3.Controls.Add(this.checkBoxBNO_reject_spaces);
            this.tabPage3.Controls.Add(this.textBoxBNOblacklist);
            this.tabPage3.Controls.Add(this.label5);
            this.tabPage3.Controls.Add(this.textBoxBNOwhitelist);
            this.tabPage3.Controls.Add(this.label4);
            this.tabPage3.Controls.Add(this.label3);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(748, 349);
            this.tabPage3.TabIndex = 0;
            this.tabPage3.Tag = "BNO";
            this.tabPage3.Text = "XML/JSON files";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // label23
            // 
            this.label23.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label23.AutoSize = true;
            this.label23.Enabled = false;
            this.label23.Font = new System.Drawing.Font("DejaVu Sans", 66F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label23.ForeColor = System.Drawing.SystemColors.ScrollBar;
            this.label23.Location = new System.Drawing.Point(502, 243);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(260, 103);
            this.label23.TabIndex = 4;
            this.label23.Text = "BNO";
            // 
            // checkBoxBNO_reject_spaces
            // 
            this.checkBoxBNO_reject_spaces.AutoSize = true;
            this.checkBoxBNO_reject_spaces.Location = new System.Drawing.Point(9, 103);
            this.checkBoxBNO_reject_spaces.Name = "checkBoxBNO_reject_spaces";
            this.checkBoxBNO_reject_spaces.Size = new System.Drawing.Size(184, 17);
            this.checkBoxBNO_reject_spaces.TabIndex = 3;
            this.checkBoxBNO_reject_spaces.Tag = "no-spaces";
            this.checkBoxBNO_reject_spaces.Text = "Ignore all whitespaced text nodes";
            this.toolTip.SetToolTip(this.checkBoxBNO_reject_spaces, "Reject all whitespaced text nodes");
            this.checkBoxBNO_reject_spaces.UseVisualStyleBackColor = true;
            // 
            // textBoxBNOblacklist
            // 
            this.textBoxBNOblacklist.Location = new System.Drawing.Point(62, 77);
            this.textBoxBNOblacklist.Name = "textBoxBNOblacklist";
            this.textBoxBNOblacklist.Size = new System.Drawing.Size(680, 20);
            this.textBoxBNOblacklist.TabIndex = 2;
            this.textBoxBNOblacklist.Tag = "blacklist";
            this.toolTip.SetToolTip(this.textBoxBNOblacklist, "Nodes to disallow whitespaced text, each tag name is separated by \':\' character");
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 80);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(49, 13);
            this.label5.TabIndex = 1;
            this.label5.Text = "Blacklist:";
            // 
            // textBoxBNOwhitelist
            // 
            this.textBoxBNOwhitelist.Location = new System.Drawing.Point(62, 51);
            this.textBoxBNOwhitelist.Name = "textBoxBNOwhitelist";
            this.textBoxBNOwhitelist.Size = new System.Drawing.Size(680, 20);
            this.textBoxBNOwhitelist.TabIndex = 2;
            this.textBoxBNOwhitelist.Tag = "whitelist";
            this.textBoxBNOwhitelist.Text = "String:Lua:VertexSource:FragmentSource:Title";
            this.toolTip.SetToolTip(this.textBoxBNOwhitelist, "Nodes to allow whitespaced text, each tag name is separated by \':\' character");
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 54);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(50, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Whitelist:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(6, 16);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(335, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Convert *.json and *.xml files into *.bno format (Binary Notation Object)";
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.groupBox2);
            this.tabPage4.Controls.Add(this.groupBox1);
            this.tabPage4.Controls.Add(this.label24);
            this.tabPage4.Controls.Add(this.groupBox6);
            this.tabPage4.Controls.Add(this.groupBox5);
            this.tabPage4.Controls.Add(this.groupBox4);
            this.tabPage4.Controls.Add(this.groupBox3);
            this.tabPage4.Controls.Add(this.label6);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(748, 349);
            this.tabPage4.TabIndex = 1;
            this.tabPage4.Tag = "KDT";
            this.tabPage4.Text = "Images and textures";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.checkBoxKDT_opacity_slice);
            this.groupBox2.Controls.Add(this.numericUpDown1);
            this.groupBox2.Controls.Add(this.label32);
            this.groupBox2.Controls.Add(this.checkBoxKDT_sub_slice);
            this.groupBox2.Controls.Add(this.checkBoxKDT_uper_slice);
            this.groupBox2.Location = new System.Drawing.Point(9, 226);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(375, 65);
            this.groupBox2.TabIndex = 23;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Slice options";
            // 
            // checkBoxKDT_opacity_slice
            // 
            this.checkBoxKDT_opacity_slice.AutoSize = true;
            this.checkBoxKDT_opacity_slice.Location = new System.Drawing.Point(208, 42);
            this.checkBoxKDT_opacity_slice.Name = "checkBoxKDT_opacity_slice";
            this.checkBoxKDT_opacity_slice.Size = new System.Drawing.Size(146, 17);
            this.checkBoxKDT_opacity_slice.TabIndex = 4;
            this.checkBoxKDT_opacity_slice.Tag = "opacity-slice";
            this.checkBoxKDT_opacity_slice.Text = "Slice right/bottom opacity";
            this.toolTip.SetToolTip(this.checkBoxKDT_opacity_slice, "Ignores right and bottom \'transparency padding\' if exists, this can reduce VRAM u" +
        "sage on power-of-two spritesheets.");
            this.checkBoxKDT_opacity_slice.UseVisualStyleBackColor = true;
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Location = new System.Drawing.Point(323, 18);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            3,
            0,
            0,
            0});
            this.numericUpDown1.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(45, 20);
            this.numericUpDown1.TabIndex = 2;
            this.numericUpDown1.Tag = "uper-slice-max-blocks";
            this.toolTip.SetToolTip(this.numericUpDown1, "Maximum slice frame dimmension, used with \'--uper-slice\'. default=3 minimum=2(204" +
        "8x2048) maximum=3(3072x3072)");
            this.numericUpDown1.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // label32
            // 
            this.label32.AutoSize = true;
            this.label32.Location = new System.Drawing.Point(205, 20);
            this.label32.Name = "label32";
            this.label32.Size = new System.Drawing.Size(112, 13);
            this.label32.TabIndex = 1;
            this.label32.Text = "Maximum slice blocks:";
            // 
            // checkBoxKDT_sub_slice
            // 
            this.checkBoxKDT_sub_slice.AutoSize = true;
            this.checkBoxKDT_sub_slice.Location = new System.Drawing.Point(9, 42);
            this.checkBoxKDT_sub_slice.Name = "checkBoxKDT_sub_slice";
            this.checkBoxKDT_sub_slice.Size = new System.Drawing.Size(169, 17);
            this.checkBoxKDT_sub_slice.TabIndex = 3;
            this.checkBoxKDT_sub_slice.Tag = "sub-slice";
            this.checkBoxKDT_sub_slice.Text = "Subslice low-resolution images";
            this.toolTip.SetToolTip(this.checkBoxKDT_sub_slice, "Allow slice right-bottom image borders in order to add less frame padding, this r" +
        "educes VRAM usage. Can be used with \'--uper-slice\' option");
            this.checkBoxKDT_sub_slice.UseVisualStyleBackColor = true;
            // 
            // checkBoxKDT_uper_slice
            // 
            this.checkBoxKDT_uper_slice.AutoSize = true;
            this.checkBoxKDT_uper_slice.Location = new System.Drawing.Point(9, 18);
            this.checkBoxKDT_uper_slice.Name = "checkBoxKDT_uper_slice";
            this.checkBoxKDT_uper_slice.Size = new System.Drawing.Size(177, 17);
            this.checkBoxKDT_uper_slice.TabIndex = 0;
            this.checkBoxKDT_uper_slice.Tag = "uper-slice";
            this.checkBoxKDT_uper_slice.Text = "Uperslice high-resolution images";
            this.toolTip.SetToolTip(this.checkBoxKDT_uper_slice, "Store high-resolution images (bigger than 1024x1024) as encoded chunks, this redu" +
        "ces the amount of downscaling needed but requires more VRAM.");
            this.checkBoxKDT_uper_slice.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.textBoxKDT_image_magick_exec);
            this.groupBox1.Controls.Add(this.label30);
            this.groupBox1.Location = new System.Drawing.Point(9, 297);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(375, 44);
            this.groupBox1.TabIndex = 22;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "ICO/DDS options";
            this.groupBox1.Visible = false;
            // 
            // textBoxKDT_image_magick_exec
            // 
            this.textBoxKDT_image_magick_exec.Location = new System.Drawing.Point(141, 17);
            this.textBoxKDT_image_magick_exec.Name = "textBoxKDT_image_magick_exec";
            this.textBoxKDT_image_magick_exec.Size = new System.Drawing.Size(227, 20);
            this.textBoxKDT_image_magick_exec.TabIndex = 1;
            this.textBoxKDT_image_magick_exec.Tag = "image-magick-exec";
            this.toolTip.SetToolTip(this.textBoxKDT_image_magick_exec, "Path to ImageMagick executable, only needed for decoding dds and ico files. Defau" +
        "lt: convert.exe or magick.exe");
            // 
            // label30
            // 
            this.label30.AutoSize = true;
            this.label30.Location = new System.Drawing.Point(6, 20);
            this.label30.Name = "label30";
            this.label30.Size = new System.Drawing.Size(129, 13);
            this.label30.TabIndex = 0;
            this.label30.Text = "ImageMagick executable:";
            // 
            // label24
            // 
            this.label24.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label24.AutoSize = true;
            this.label24.Enabled = false;
            this.label24.Font = new System.Drawing.Font("DejaVu Sans Condensed", 66F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label24.ForeColor = System.Drawing.SystemColors.ScrollBar;
            this.label24.Location = new System.Drawing.Point(537, 243);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(225, 103);
            this.label24.TabIndex = 21;
            this.label24.Text = "KDT";
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.label33);
            this.groupBox6.Controls.Add(this.numericUpDownKDT_max_dimmen);
            this.groupBox6.Controls.Add(this.label12);
            this.groupBox6.Controls.Add(this.label13);
            this.groupBox6.Controls.Add(this.numericUpDownKDT_scale_factor);
            this.groupBox6.Controls.Add(this.numericUpDownKDT_scale_factor_limit);
            this.groupBox6.Location = new System.Drawing.Point(390, 119);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(352, 101);
            this.groupBox6.TabIndex = 20;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Advanced resize options";
            // 
            // label33
            // 
            this.label33.AutoSize = true;
            this.label33.Location = new System.Drawing.Point(6, 74);
            this.label33.Name = "label33";
            this.label33.Size = new System.Drawing.Size(112, 13);
            this.label33.TabIndex = 18;
            this.label33.Text = "Maximum dimmension:";
            // 
            // numericUpDownKDT_max_dimmen
            // 
            this.numericUpDownKDT_max_dimmen.Increment = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDownKDT_max_dimmen.Location = new System.Drawing.Point(127, 72);
            this.numericUpDownKDT_max_dimmen.Maximum = new decimal(new int[] {
            3072,
            0,
            0,
            0});
            this.numericUpDownKDT_max_dimmen.Name = "numericUpDownKDT_max_dimmen";
            this.numericUpDownKDT_max_dimmen.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDT_max_dimmen.TabIndex = 17;
            this.numericUpDownKDT_max_dimmen.Tag = "max-dimmen";
            this.toolTip.SetToolTip(this.numericUpDownKDT_max_dimmen, resources.GetString("numericUpDownKDT_max_dimmen.ToolTip"));
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(6, 22);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(103, 13);
            this.label12.TabIndex = 15;
            this.label12.Text = "Manual scale factor:";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(6, 48);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(87, 13);
            this.label13.TabIndex = 15;
            this.label13.Text = "Scale factor limit:";
            // 
            // numericUpDownKDT_scale_factor
            // 
            this.numericUpDownKDT_scale_factor.DecimalPlaces = 4;
            this.numericUpDownKDT_scale_factor.Location = new System.Drawing.Point(127, 20);
            this.numericUpDownKDT_scale_factor.Maximum = new decimal(new int[] {
            16,
            0,
            0,
            0});
            this.numericUpDownKDT_scale_factor.Name = "numericUpDownKDT_scale_factor";
            this.numericUpDownKDT_scale_factor.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDT_scale_factor.TabIndex = 16;
            this.numericUpDownKDT_scale_factor.Tag = "scale-factor";
            this.toolTip.SetToolTip(this.numericUpDownKDT_scale_factor, "Ignores the downscale procedure and resize with the specified amount of times. De" +
        "fault: 0.0");
            // 
            // numericUpDownKDT_scale_factor_limit
            // 
            this.numericUpDownKDT_scale_factor_limit.Location = new System.Drawing.Point(127, 46);
            this.numericUpDownKDT_scale_factor_limit.Maximum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.numericUpDownKDT_scale_factor_limit.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownKDT_scale_factor_limit.Name = "numericUpDownKDT_scale_factor_limit";
            this.numericUpDownKDT_scale_factor_limit.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDT_scale_factor_limit.TabIndex = 16;
            this.numericUpDownKDT_scale_factor_limit.Tag = "scale-factor-limit";
            this.toolTip.SetToolTip(this.numericUpDownKDT_scale_factor_limit, "Do not resize if the image size is less than the specified dimmension, used with " +
        "\"Manual scale factor\". Default: 16");
            this.numericUpDownKDT_scale_factor_limit.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.label10);
            this.groupBox5.Controls.Add(this.comboBoxKDT_downscale_procedure);
            this.groupBox5.Controls.Add(this.comboBoxKDT_pixel_format);
            this.groupBox5.Controls.Add(this.label11);
            this.groupBox5.Location = new System.Drawing.Point(390, 39);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(352, 74);
            this.groupBox5.TabIndex = 19;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Advanced";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(6, 21);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(114, 13);
            this.label10.TabIndex = 13;
            this.label10.Text = "Downscale procedure:";
            // 
            // comboBoxKDT_downscale_procedure
            // 
            this.comboBoxKDT_downscale_procedure.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDT_downscale_procedure.FormattingEnabled = true;
            this.comboBoxKDT_downscale_procedure.Items.AddRange(new object[] {
            "CLAMP (default)",
            "MEDIUM",
            "HARD"});
            this.comboBoxKDT_downscale_procedure.Location = new System.Drawing.Point(127, 18);
            this.comboBoxKDT_downscale_procedure.Name = "comboBoxKDT_downscale_procedure";
            this.comboBoxKDT_downscale_procedure.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDT_downscale_procedure.TabIndex = 7;
            this.comboBoxKDT_downscale_procedure.Tag = "downscale-procedure";
            this.toolTip.SetToolTip(this.comboBoxKDT_downscale_procedure, resources.GetString("comboBoxKDT_downscale_procedure.ToolTip"));
            // 
            // comboBoxKDT_pixel_format
            // 
            this.comboBoxKDT_pixel_format.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDT_pixel_format.FormattingEnabled = true;
            this.comboBoxKDT_pixel_format.Items.AddRange(new object[] {
            "(auto choose)",
            "YUV422",
            "RGB565",
            "ARGB1555",
            "ARGB4444"});
            this.comboBoxKDT_pixel_format.Location = new System.Drawing.Point(127, 45);
            this.comboBoxKDT_pixel_format.Name = "comboBoxKDT_pixel_format";
            this.comboBoxKDT_pixel_format.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDT_pixel_format.TabIndex = 7;
            this.comboBoxKDT_pixel_format.Tag = "pixel-format";
            this.toolTip.SetToolTip(this.comboBoxKDT_pixel_format, resources.GetString("comboBoxKDT_pixel_format.ToolTip"));
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(6, 48);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(64, 13);
            this.label11.TabIndex = 14;
            this.label11.Text = "Pixel format:";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.checkBoxKDT_force_vq_on_small);
            this.groupBox4.Controls.Add(this.checkBoxKDT_force_square_vq);
            this.groupBox4.Location = new System.Drawing.Point(9, 175);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(375, 45);
            this.groupBox4.TabIndex = 18;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Specific VQ options ( not compatible with slice)";
            // 
            // checkBoxKDT_force_vq_on_small
            // 
            this.checkBoxKDT_force_vq_on_small.AutoSize = true;
            this.checkBoxKDT_force_vq_on_small.Location = new System.Drawing.Point(9, 19);
            this.checkBoxKDT_force_vq_on_small.Name = "checkBoxKDT_force_vq_on_small";
            this.checkBoxKDT_force_vq_on_small.Size = new System.Drawing.Size(192, 17);
            this.checkBoxKDT_force_vq_on_small.TabIndex = 11;
            this.checkBoxKDT_force_vq_on_small.Tag = "force-vq-on-small";
            this.checkBoxKDT_force_vq_on_small.Text = "Force compression for small images";
            this.toolTip.SetToolTip(this.checkBoxKDT_force_vq_on_small, "Forces vector quantization on textures smaller than 64x64, waste of space.");
            this.checkBoxKDT_force_vq_on_small.UseVisualStyleBackColor = true;
            // 
            // checkBoxKDT_force_square_vq
            // 
            this.checkBoxKDT_force_square_vq.AutoSize = true;
            this.checkBoxKDT_force_square_vq.Location = new System.Drawing.Point(218, 19);
            this.checkBoxKDT_force_square_vq.Name = "checkBoxKDT_force_square_vq";
            this.checkBoxKDT_force_square_vq.Size = new System.Drawing.Size(151, 17);
            this.checkBoxKDT_force_square_vq.TabIndex = 12;
            this.checkBoxKDT_force_square_vq.Tag = "force-square-vq";
            this.checkBoxKDT_force_square_vq.Text = "Force square dimmensions";
            this.toolTip.SetToolTip(this.checkBoxKDT_force_square_vq, "Always wrap texture with square dimmensions (example 128x128), applies only if ve" +
        "ctor quantization is needed");
            this.checkBoxKDT_force_square_vq.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.checkBoxKDT_lzss);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.numericUpDownKDT_quality);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Controls.Add(this.comboBoxKDT_dither_algorithm);
            this.groupBox3.Controls.Add(this.label9);
            this.groupBox3.Controls.Add(this.comboBoxKDT_scale_agorithm);
            this.groupBox3.Controls.Add(this.checkBoxKDT_rgb565);
            this.groupBox3.Controls.Add(this.checkBoxKDT_vq);
            this.groupBox3.Controls.Add(this.checkBoxKDT_no_twiddled);
            this.groupBox3.Location = new System.Drawing.Point(9, 39);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(375, 130);
            this.groupBox3.TabIndex = 17;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Common";
            // 
            // checkBoxKDT_lzss
            // 
            this.checkBoxKDT_lzss.AutoSize = true;
            this.checkBoxKDT_lzss.Location = new System.Drawing.Point(237, 20);
            this.checkBoxKDT_lzss.Name = "checkBoxKDT_lzss";
            this.checkBoxKDT_lzss.Size = new System.Drawing.Size(131, 17);
            this.checkBoxKDT_lzss.TabIndex = 11;
            this.checkBoxKDT_lzss.Tag = "lzss";
            this.checkBoxKDT_lzss.Text = "LZSS file compression";
            this.toolTip.SetToolTip(this.checkBoxKDT_lzss, "Compress the file with LZSS, only improves read and parse times. Does not affect " +
        "the VRAM usage");
            this.checkBoxKDT_lzss.UseVisualStyleBackColor = true;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(6, 21);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(42, 13);
            this.label7.TabIndex = 2;
            this.label7.Text = "Quality:";
            // 
            // numericUpDownKDT_quality
            // 
            this.numericUpDownKDT_quality.Location = new System.Drawing.Point(103, 19);
            this.numericUpDownKDT_quality.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.numericUpDownKDT_quality.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownKDT_quality.Name = "numericUpDownKDT_quality";
            this.numericUpDownKDT_quality.Size = new System.Drawing.Size(110, 20);
            this.numericUpDownKDT_quality.TabIndex = 3;
            this.numericUpDownKDT_quality.Tag = "quality";
            this.toolTip.SetToolTip(this.numericUpDownKDT_quality, "Vector quantization steps, higher values allows better quality. Default: 500");
            this.numericUpDownKDT_quality.Value = new decimal(new int[] {
            500,
            0,
            0,
            0});
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(6, 48);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(83, 13);
            this.label8.TabIndex = 4;
            this.label8.Text = "Dither algorithm:";
            // 
            // comboBoxKDT_dither_algorithm
            // 
            this.comboBoxKDT_dither_algorithm.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDT_dither_algorithm.FormattingEnabled = true;
            this.comboBoxKDT_dither_algorithm.Items.AddRange(new object[] {
            "(auto choose)",
            "bayer dither",
            "error diffusion",
            "arithmetic addition dither",
            "arithmetic xor dither"});
            this.comboBoxKDT_dither_algorithm.Location = new System.Drawing.Point(102, 45);
            this.comboBoxKDT_dither_algorithm.Name = "comboBoxKDT_dither_algorithm";
            this.comboBoxKDT_dither_algorithm.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDT_dither_algorithm.TabIndex = 5;
            this.comboBoxKDT_dither_algorithm.Tag = "dither-algorithm";
            this.toolTip.SetToolTip(this.comboBoxKDT_dither_algorithm, "Dither algortihm used for pixel format conversion. Default: auto");
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(6, 79);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(82, 13);
            this.label9.TabIndex = 6;
            this.label9.Text = "Scale algorithm:";
            // 
            // comboBoxKDT_scale_agorithm
            // 
            this.comboBoxKDT_scale_agorithm.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDT_scale_agorithm.FormattingEnabled = true;
            this.comboBoxKDT_scale_agorithm.Items.AddRange(new object[] {
            "fast bilinear",
            "bilinear",
            "bicubic",
            "experimental",
            "nearest neighbor",
            "averaging area",
            "luma bicubic, chroma bilinear",
            "Gaussian",
            "sinc",
            "Lanczos (default)",
            "natural bicubic spline",
            "print info",
            "accurate rounding",
            "full chroma interpolation",
            "full chroma input",
            "bitexact",
            "error diffusion dither"});
            this.comboBoxKDT_scale_agorithm.Location = new System.Drawing.Point(102, 76);
            this.comboBoxKDT_scale_agorithm.Name = "comboBoxKDT_scale_agorithm";
            this.comboBoxKDT_scale_agorithm.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDT_scale_agorithm.TabIndex = 7;
            this.comboBoxKDT_scale_agorithm.Tag = "scale-algorithm";
            this.toolTip.SetToolTip(this.comboBoxKDT_scale_agorithm, "Used only if the image needs downscaling. Default: lanczos");
            // 
            // checkBoxKDT_rgb565
            // 
            this.checkBoxKDT_rgb565.AutoSize = true;
            this.checkBoxKDT_rgb565.Location = new System.Drawing.Point(9, 103);
            this.checkBoxKDT_rgb565.Name = "checkBoxKDT_rgb565";
            this.checkBoxKDT_rgb565.Size = new System.Drawing.Size(123, 17);
            this.checkBoxKDT_rgb565.TabIndex = 8;
            this.checkBoxKDT_rgb565.Tag = "rgb565";
            this.checkBoxKDT_rgb565.Text = "RGB565 pixel format";
            this.toolTip.SetToolTip(this.checkBoxKDT_rgb565, "Use RGB565 pixel format instead of YUV422, this can reduce artifacts but produces" +
        " color banding.");
            this.checkBoxKDT_rgb565.UseVisualStyleBackColor = true;
            // 
            // checkBoxKDT_vq
            // 
            this.checkBoxKDT_vq.AutoSize = true;
            this.checkBoxKDT_vq.Checked = true;
            this.checkBoxKDT_vq.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxKDT_vq.Location = new System.Drawing.Point(138, 103);
            this.checkBoxKDT_vq.Name = "checkBoxKDT_vq";
            this.checkBoxKDT_vq.Size = new System.Drawing.Size(103, 17);
            this.checkBoxKDT_vq.TabIndex = 9;
            this.checkBoxKDT_vq.Tag = "vq";
            this.checkBoxKDT_vq.Text = "VQ compression";
            this.toolTip.SetToolTip(this.checkBoxKDT_vq, "Compress texture using vector quantization, lossy format but saves a lot of VRAM." +
        "");
            this.checkBoxKDT_vq.UseVisualStyleBackColor = true;
            // 
            // checkBoxKDT_no_twiddled
            // 
            this.checkBoxKDT_no_twiddled.AutoSize = true;
            this.checkBoxKDT_no_twiddled.Location = new System.Drawing.Point(247, 103);
            this.checkBoxKDT_no_twiddled.Name = "checkBoxKDT_no_twiddled";
            this.checkBoxKDT_no_twiddled.Size = new System.Drawing.Size(97, 17);
            this.checkBoxKDT_no_twiddled.TabIndex = 10;
            this.checkBoxKDT_no_twiddled.Tag = "no-twiddled";
            this.checkBoxKDT_no_twiddled.Text = "Disable twiddle";
            this.toolTip.SetToolTip(this.checkBoxKDT_no_twiddled, "Disable pixel twiddling, this decreases rendering performance.");
            this.checkBoxKDT_no_twiddled.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(6, 16);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(371, 13);
            this.label6.TabIndex = 1;
            this.label6.Text = "Convert images (*.png, *.jpg and *.ico files) to *.kdt (KDY-e dreamcast texture)";
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.label25);
            this.tabPage5.Controls.Add(this.groupBox8);
            this.tabPage5.Controls.Add(this.groupBox7);
            this.tabPage5.Controls.Add(this.label14);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(748, 349);
            this.tabPage5.TabIndex = 2;
            this.tabPage5.Tag = "KDM";
            this.tabPage5.Text = "Videos";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // label25
            // 
            this.label25.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label25.AutoSize = true;
            this.label25.Enabled = false;
            this.label25.Font = new System.Drawing.Font("DejaVu Sans", 66F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label25.ForeColor = System.Drawing.SystemColors.ScrollBar;
            this.label25.Location = new System.Drawing.Point(469, 243);
            this.label25.Name = "label25";
            this.label25.Size = new System.Drawing.Size(273, 103);
            this.label25.TabIndex = 5;
            this.label25.Text = "KDM";
            // 
            // groupBox8
            // 
            this.groupBox8.Controls.Add(this.checkBoxKDM_mono);
            this.groupBox8.Controls.Add(this.numericUpDownKDM_sample_rate);
            this.groupBox8.Controls.Add(this.label19);
            this.groupBox8.Controls.Add(this.checkBoxKDM_silence);
            this.groupBox8.Location = new System.Drawing.Point(367, 57);
            this.groupBox8.Name = "groupBox8";
            this.groupBox8.Size = new System.Drawing.Size(375, 119);
            this.groupBox8.TabIndex = 4;
            this.groupBox8.TabStop = false;
            this.groupBox8.Text = "Audio options";
            // 
            // checkBoxKDM_mono
            // 
            this.checkBoxKDM_mono.AutoSize = true;
            this.checkBoxKDM_mono.Location = new System.Drawing.Point(11, 77);
            this.checkBoxKDM_mono.Name = "checkBoxKDM_mono";
            this.checkBoxKDM_mono.Size = new System.Drawing.Size(53, 17);
            this.checkBoxKDM_mono.TabIndex = 31;
            this.checkBoxKDM_mono.Tag = "mono";
            this.checkBoxKDM_mono.Text = "Mono";
            this.toolTip.SetToolTip(this.checkBoxKDM_mono, "Downmix the audio stream to mono.");
            this.checkBoxKDM_mono.UseVisualStyleBackColor = true;
            // 
            // numericUpDownKDM_sample_rate
            // 
            this.numericUpDownKDM_sample_rate.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownKDM_sample_rate.Location = new System.Drawing.Point(80, 47);
            this.numericUpDownKDM_sample_rate.Maximum = new decimal(new int[] {
            48000,
            0,
            0,
            0});
            this.numericUpDownKDM_sample_rate.Name = "numericUpDownKDM_sample_rate";
            this.numericUpDownKDM_sample_rate.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDM_sample_rate.TabIndex = 30;
            this.numericUpDownKDM_sample_rate.Tag = "sample-rate";
            this.toolTip.SetToolTip(this.numericUpDownKDM_sample_rate, "Audio sample rate in hertz, recommended value is 32000. By default the original s" +
        "ample rate is used.");
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(8, 49);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(66, 13);
            this.label19.TabIndex = 29;
            this.label19.Text = "Sample rate:";
            // 
            // checkBoxKDM_silence
            // 
            this.checkBoxKDM_silence.AutoSize = true;
            this.checkBoxKDM_silence.Location = new System.Drawing.Point(11, 22);
            this.checkBoxKDM_silence.Name = "checkBoxKDM_silence";
            this.checkBoxKDM_silence.Size = new System.Drawing.Size(96, 17);
            this.checkBoxKDM_silence.TabIndex = 28;
            this.checkBoxKDM_silence.Tag = "silence";
            this.checkBoxKDM_silence.Text = "No audio track";
            this.toolTip.SetToolTip(this.checkBoxKDM_silence, "Ignore the audio stream.");
            this.checkBoxKDM_silence.UseVisualStyleBackColor = true;
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.checkBoxKDM_no_progress);
            this.groupBox7.Controls.Add(this.checkBoxKDM_small_resolution);
            this.groupBox7.Controls.Add(this.checkBoxKDM_no_save_original_resolution);
            this.groupBox7.Controls.Add(this.numericUpDownKDM_gop);
            this.groupBox7.Controls.Add(this.label34);
            this.groupBox7.Controls.Add(this.comboBoxKDM_fps);
            this.groupBox7.Controls.Add(this.numericUpDownKDM_mpeg_bitrate);
            this.groupBox7.Controls.Add(this.label31);
            this.groupBox7.Controls.Add(this.numericUpDownKDM_two_pass_bitrate);
            this.groupBox7.Controls.Add(this.label15);
            this.groupBox7.Controls.Add(this.checkBoxKDM_hq);
            this.groupBox7.Controls.Add(this.label29);
            this.groupBox7.Controls.Add(this.label18);
            this.groupBox7.Controls.Add(this.numericUpDownKDM_cue_interval);
            this.groupBox7.Controls.Add(this.label16);
            this.groupBox7.Controls.Add(this.comboBoxKDM_dither_algorithm);
            this.groupBox7.Controls.Add(this.label17);
            this.groupBox7.Controls.Add(this.comboBoxKDM_scale_algorithm);
            this.groupBox7.Location = new System.Drawing.Point(6, 57);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(355, 274);
            this.groupBox7.TabIndex = 16;
            this.groupBox7.TabStop = false;
            this.groupBox7.Text = "Video options";
            // 
            // comboBoxKDM_fps
            // 
            this.comboBoxKDM_fps.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDM_fps.FormattingEnabled = true;
            this.comboBoxKDM_fps.Items.AddRange(new object[] {
            "(no change, can fail)",
            "23.976",
            "24",
            "25",
            "29.97",
            "30",
            "50",
            "59.94"});
            this.comboBoxKDM_fps.Location = new System.Drawing.Point(103, 73);
            this.comboBoxKDM_fps.Name = "comboBoxKDM_fps";
            this.comboBoxKDM_fps.Size = new System.Drawing.Size(121, 21);
            this.comboBoxKDM_fps.TabIndex = 17;
            this.comboBoxKDM_fps.Tag = "fps";
            this.toolTip.SetToolTip(this.comboBoxKDM_fps, "Frames per second, the maximum is 60. By default the original framerate is used i" +
        "f applicable");
            // 
            // checkBoxKDM_no_save_original_resolution
            // 
            this.checkBoxKDM_no_save_original_resolution.AutoSize = true;
            this.checkBoxKDM_no_save_original_resolution.Location = new System.Drawing.Point(9, 249);
            this.checkBoxKDM_no_save_original_resolution.Name = "checkBoxKDM_no_save_original_resolution";
            this.checkBoxKDM_no_save_original_resolution.Size = new System.Drawing.Size(150, 17);
            this.checkBoxKDM_no_save_original_resolution.TabIndex = 22;
            this.checkBoxKDM_no_save_original_resolution.Tag = "no-save-original-resolution";
            this.checkBoxKDM_no_save_original_resolution.Text = "No save original resolution";
            this.toolTip.SetToolTip(this.checkBoxKDM_no_save_original_resolution, "Do not store the original video resolution in the output file if was downscaled.");
            this.checkBoxKDM_no_save_original_resolution.UseVisualStyleBackColor = true;
            // 
            // numericUpDownKDM_mpeg_bitrate
            // 
            this.numericUpDownKDM_mpeg_bitrate.Location = new System.Drawing.Point(103, 177);
            this.numericUpDownKDM_mpeg_bitrate.Maximum = new decimal(new int[] {
            800000,
            0,
            0,
            0});
            this.numericUpDownKDM_mpeg_bitrate.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.numericUpDownKDM_mpeg_bitrate.Name = "numericUpDownKDM_mpeg_bitrate";
            this.numericUpDownKDM_mpeg_bitrate.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDM_mpeg_bitrate.TabIndex = 26;
            this.numericUpDownKDM_mpeg_bitrate.Tag = "mpeg-bitrate";
            this.toolTip.SetToolTip(this.numericUpDownKDM_mpeg_bitrate, "Final video bitrate expressed in Kbits/s, this determines the video quality. Defa" +
        "ult: 800 if \'--hq\' is set, otherwise, 400");
            this.numericUpDownKDM_mpeg_bitrate.Value = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            // 
            // label31
            // 
            this.label31.AutoSize = true;
            this.label31.Location = new System.Drawing.Point(6, 179);
            this.label31.Name = "label31";
            this.label31.Size = new System.Drawing.Size(73, 13);
            this.label31.TabIndex = 25;
            this.label31.Text = "MPEG bitrate:";
            // 
            // numericUpDownKDM_two_pass_bitrate
            // 
            this.numericUpDownKDM_two_pass_bitrate.Location = new System.Drawing.Point(103, 151);
            this.numericUpDownKDM_two_pass_bitrate.Maximum = new decimal(new int[] {
            800000,
            0,
            0,
            0});
            this.numericUpDownKDM_two_pass_bitrate.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownKDM_two_pass_bitrate.Name = "numericUpDownKDM_two_pass_bitrate";
            this.numericUpDownKDM_two_pass_bitrate.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownKDM_two_pass_bitrate.TabIndex = 24;
            this.numericUpDownKDM_two_pass_bitrate.Tag = "two-pass-bitrate";
            this.toolTip.SetToolTip(this.numericUpDownKDM_two_pass_bitrate, "Two-pass video encoding bitrate expressed in Kbits/s, can improve the video quali" +
        "ty. Default: 5000");
            this.numericUpDownKDM_two_pass_bitrate.Value = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(6, 153);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(88, 13);
            this.label15.TabIndex = 23;
            this.label15.Text = "Two-pass bitrate:";
            // 
            // checkBoxKDM_no_progress
            // 
            this.checkBoxKDM_no_progress.AutoSize = true;
            this.checkBoxKDM_no_progress.Checked = true;
            this.checkBoxKDM_no_progress.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxKDM_no_progress.Enabled = false;
            this.checkBoxKDM_no_progress.Location = new System.Drawing.Point(231, 249);
            this.checkBoxKDM_no_progress.Name = "checkBoxKDM_no_progress";
            this.checkBoxKDM_no_progress.Size = new System.Drawing.Size(118, 17);
            this.checkBoxKDM_no_progress.TabIndex = 27;
            this.checkBoxKDM_no_progress.Tag = "no-progress";
            this.checkBoxKDM_no_progress.Text = "No display progress";
            this.toolTip.SetToolTip(this.checkBoxKDM_no_progress, "No display encoding progress");
            this.checkBoxKDM_no_progress.UseVisualStyleBackColor = true;
            this.checkBoxKDM_no_progress.Visible = false;
            // 
            // checkBoxKDM_hq
            // 
            this.checkBoxKDM_hq.AutoSize = true;
            this.checkBoxKDM_hq.Location = new System.Drawing.Point(9, 203);
            this.checkBoxKDM_hq.Name = "checkBoxKDM_hq";
            this.checkBoxKDM_hq.Size = new System.Drawing.Size(81, 17);
            this.checkBoxKDM_hq.TabIndex = 12;
            this.checkBoxKDM_hq.Tag = "hq";
            this.checkBoxKDM_hq.Text = "High quality";
            this.toolTip.SetToolTip(this.checkBoxKDM_hq, "Uses a high bitrate, improves quality alot but can produce suttering (see notes)." +
        "\r\nCan be overrided by \"--mpeg-bitrate\" option.");
            this.checkBoxKDM_hq.UseVisualStyleBackColor = true;
            // 
            // label29
            // 
            this.label29.AutoSize = true;
            this.label29.Location = new System.Drawing.Point(6, 76);
            this.label29.Name = "label29";
            this.label29.Size = new System.Drawing.Size(57, 13);
            this.label29.TabIndex = 16;
            this.label29.Text = "Framerate:";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(6, 101);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(66, 13);
            this.label18.TabIndex = 18;
            this.label18.Text = "Cue interval:";
            // 
            // numericUpDownKDM_cue_interval
            // 
            this.numericUpDownKDM_cue_interval.Location = new System.Drawing.Point(103, 99);
            this.numericUpDownKDM_cue_interval.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.numericUpDownKDM_cue_interval.Name = "numericUpDownKDM_cue_interval";
            this.numericUpDownKDM_cue_interval.Size = new System.Drawing.Size(56, 20);
            this.numericUpDownKDM_cue_interval.TabIndex = 19;
            this.numericUpDownKDM_cue_interval.Tag = "cue-interval";
            this.toolTip.SetToolTip(this.numericUpDownKDM_cue_interval, "Used for seeking, lower values allow fast seeking. Default: 3");
            this.numericUpDownKDM_cue_interval.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(6, 22);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(83, 13);
            this.label16.TabIndex = 13;
            this.label16.Text = "Dither algorithm:";
            // 
            // comboBoxKDM_dither_algorithm
            // 
            this.comboBoxKDM_dither_algorithm.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDM_dither_algorithm.FormattingEnabled = true;
            this.comboBoxKDM_dither_algorithm.Items.AddRange(new object[] {
            "(auto choose)",
            "bayer dither",
            "error diffusion",
            "arithmetic addition dither",
            "arithmetic xor dither"});
            this.comboBoxKDM_dither_algorithm.Location = new System.Drawing.Point(103, 19);
            this.comboBoxKDM_dither_algorithm.Name = "comboBoxKDM_dither_algorithm";
            this.comboBoxKDM_dither_algorithm.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDM_dither_algorithm.TabIndex = 14;
            this.comboBoxKDM_dither_algorithm.Tag = "dither-algorithm";
            this.toolTip.SetToolTip(this.comboBoxKDM_dither_algorithm, "Dither algortihm used for RGB565/YUV422 pixel format conversion. Default: auto");
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(6, 49);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(83, 13);
            this.label17.TabIndex = 15;
            this.label17.Text = "Scale Algorithm:";
            // 
            // comboBoxKDM_scale_algorithm
            // 
            this.comboBoxKDM_scale_algorithm.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxKDM_scale_algorithm.FormattingEnabled = true;
            this.comboBoxKDM_scale_algorithm.Items.AddRange(new object[] {
            "fast bilinear",
            "bilinear",
            "bicubic",
            "experimental",
            "nearest neighbor",
            "averaging area",
            "luma bicubic, chroma bilinear",
            "Gaussian",
            "sinc",
            "Lanczos (default)",
            "natural bicubic spline",
            "print info",
            "accurate rounding",
            "full chroma interpolation",
            "full chroma input",
            "bitexact",
            "error diffusion dither"});
            this.comboBoxKDM_scale_algorithm.Location = new System.Drawing.Point(103, 46);
            this.comboBoxKDM_scale_algorithm.Name = "comboBoxKDM_scale_algorithm";
            this.comboBoxKDM_scale_algorithm.Size = new System.Drawing.Size(181, 21);
            this.comboBoxKDM_scale_algorithm.TabIndex = 16;
            this.comboBoxKDM_scale_algorithm.Tag = "scale-algorithm";
            this.toolTip.SetToolTip(this.comboBoxKDM_scale_algorithm, "Used only for downscaling. Default: lanczos");
            // 
            // checkBoxKDM_small_resolution
            // 
            this.checkBoxKDM_small_resolution.AutoSize = true;
            this.checkBoxKDM_small_resolution.Location = new System.Drawing.Point(9, 226);
            this.checkBoxKDM_small_resolution.Name = "checkBoxKDM_small_resolution";
            this.checkBoxKDM_small_resolution.Size = new System.Drawing.Size(128, 17);
            this.checkBoxKDM_small_resolution.TabIndex = 20;
            this.checkBoxKDM_small_resolution.Tag = "small-resolution";
            this.checkBoxKDM_small_resolution.Text = "Small video resolution";
            this.toolTip.SetToolTip(this.checkBoxKDM_small_resolution, "Force downscale the video to 320x240 (4:3) or 480x270 (16:9), resolution choosen " +
        "by aspect ratio");
            this.checkBoxKDM_small_resolution.UseVisualStyleBackColor = true;
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label14.Location = new System.Drawing.Point(6, 16);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(362, 13);
            this.label14.TabIndex = 2;
            this.label14.Text = "Convert videos (*.webm and *.mp4 files) to *.kdm (KDY-e Dreamcast Media)";
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.label26);
            this.tabPage6.Controls.Add(this.checkBoxSFX_test_only);
            this.tabPage6.Controls.Add(this.checkBoxSFX_copy_if_rejected);
            this.tabPage6.Controls.Add(this.checkBoxSFX_force_mono);
            this.tabPage6.Controls.Add(this.checkBoxSFX_pcm_u8);
            this.tabPage6.Controls.Add(this.checkBoxSFX_auto_sample_rate);
            this.tabPage6.Controls.Add(this.label22);
            this.tabPage6.Controls.Add(this.numericUpDownSFX_sample_rate);
            this.tabPage6.Controls.Add(this.numericUpDownSFX_maximum_duration);
            this.tabPage6.Controls.Add(this.label21);
            this.tabPage6.Controls.Add(this.label20);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage6.Size = new System.Drawing.Size(748, 349);
            this.tabPage6.TabIndex = 3;
            this.tabPage6.Tag = "SFX";
            this.tabPage6.Text = "Sound effects";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // label26
            // 
            this.label26.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label26.AutoSize = true;
            this.label26.Enabled = false;
            this.label26.Font = new System.Drawing.Font("DejaVu Sans", 66F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label26.ForeColor = System.Drawing.SystemColors.ScrollBar;
            this.label26.Location = new System.Drawing.Point(527, 243);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(235, 103);
            this.label26.TabIndex = 13;
            this.label26.Text = "SFX";
            // 
            // checkBoxSFX_test_only
            // 
            this.checkBoxSFX_test_only.AutoSize = true;
            this.checkBoxSFX_test_only.Location = new System.Drawing.Point(9, 201);
            this.checkBoxSFX_test_only.Name = "checkBoxSFX_test_only";
            this.checkBoxSFX_test_only.Size = new System.Drawing.Size(69, 17);
            this.checkBoxSFX_test_only.TabIndex = 12;
            this.checkBoxSFX_test_only.Tag = "test-only";
            this.checkBoxSFX_test_only.Text = "Test only";
            this.toolTip.SetToolTip(this.checkBoxSFX_test_only, "Display the output metadata (channels, samples, format and duration) without writ" +
        "ing any file.");
            this.checkBoxSFX_test_only.UseVisualStyleBackColor = true;
            // 
            // checkBoxSFX_copy_if_rejected
            // 
            this.checkBoxSFX_copy_if_rejected.AutoSize = true;
            this.checkBoxSFX_copy_if_rejected.Checked = true;
            this.checkBoxSFX_copy_if_rejected.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxSFX_copy_if_rejected.Location = new System.Drawing.Point(9, 178);
            this.checkBoxSFX_copy_if_rejected.Name = "checkBoxSFX_copy_if_rejected";
            this.checkBoxSFX_copy_if_rejected.Size = new System.Drawing.Size(115, 17);
            this.checkBoxSFX_copy_if_rejected.TabIndex = 11;
            this.checkBoxSFX_copy_if_rejected.Tag = "copy-if-rejected";
            this.checkBoxSFX_copy_if_rejected.Text = "Copy file if rejected";
            this.toolTip.SetToolTip(this.checkBoxSFX_copy_if_rejected, "Copies the file in the output folder as-is if the sound is not eligible.");
            this.checkBoxSFX_copy_if_rejected.UseVisualStyleBackColor = true;
            // 
            // checkBoxSFX_force_mono
            // 
            this.checkBoxSFX_force_mono.AutoSize = true;
            this.checkBoxSFX_force_mono.Location = new System.Drawing.Point(9, 155);
            this.checkBoxSFX_force_mono.Name = "checkBoxSFX_force_mono";
            this.checkBoxSFX_force_mono.Size = new System.Drawing.Size(53, 17);
            this.checkBoxSFX_force_mono.TabIndex = 10;
            this.checkBoxSFX_force_mono.Tag = "force-mono";
            this.checkBoxSFX_force_mono.Text = "Mono";
            this.toolTip.SetToolTip(this.checkBoxSFX_force_mono, "Downmix to mono, by default always is downmixed to stereo.");
            this.checkBoxSFX_force_mono.UseVisualStyleBackColor = true;
            // 
            // checkBoxSFX_pcm_u8
            // 
            this.checkBoxSFX_pcm_u8.AutoSize = true;
            this.checkBoxSFX_pcm_u8.Location = new System.Drawing.Point(9, 132);
            this.checkBoxSFX_pcm_u8.Name = "checkBoxSFX_pcm_u8";
            this.checkBoxSFX_pcm_u8.Size = new System.Drawing.Size(178, 17);
            this.checkBoxSFX_pcm_u8.TabIndex = 9;
            this.checkBoxSFX_pcm_u8.Tag = "pcm-u8";
            this.checkBoxSFX_pcm_u8.Text = "Use PCM-U8 instead of ADPCM";
            this.toolTip.SetToolTip(this.checkBoxSFX_pcm_u8, "Use \'unsigned 8-bit PCM\', better audio quality but increases x2 the result file s" +
        "ize.");
            this.checkBoxSFX_pcm_u8.UseVisualStyleBackColor = true;
            // 
            // checkBoxSFX_auto_sample_rate
            // 
            this.checkBoxSFX_auto_sample_rate.AutoSize = true;
            this.checkBoxSFX_auto_sample_rate.Location = new System.Drawing.Point(9, 109);
            this.checkBoxSFX_auto_sample_rate.Name = "checkBoxSFX_auto_sample_rate";
            this.checkBoxSFX_auto_sample_rate.Size = new System.Drawing.Size(362, 17);
            this.checkBoxSFX_auto_sample_rate.TabIndex = 8;
            this.checkBoxSFX_auto_sample_rate.Tag = "auto-sample-rate";
            this.checkBoxSFX_auto_sample_rate.Text = "Automatically choose the sample rate (previous value sets the minimum)";
            this.toolTip.SetToolTip(this.checkBoxSFX_auto_sample_rate, "Reduce sample rate until all encoding criterias are met, but not below \'--sample-" +
        "rate\' value.");
            this.checkBoxSFX_auto_sample_rate.UseVisualStyleBackColor = true;
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(6, 85);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(148, 13);
            this.label22.TabIndex = 6;
            this.label22.Text = "Sample rate (use -1 to ignore):";
            // 
            // numericUpDownSFX_sample_rate
            // 
            this.numericUpDownSFX_sample_rate.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownSFX_sample_rate.Location = new System.Drawing.Point(183, 83);
            this.numericUpDownSFX_sample_rate.Maximum = new decimal(new int[] {
            48000,
            0,
            0,
            0});
            this.numericUpDownSFX_sample_rate.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.numericUpDownSFX_sample_rate.Name = "numericUpDownSFX_sample_rate";
            this.numericUpDownSFX_sample_rate.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownSFX_sample_rate.TabIndex = 7;
            this.numericUpDownSFX_sample_rate.Tag = "sample-rate";
            this.toolTip.SetToolTip(this.numericUpDownSFX_sample_rate, "Sample rate frequency, use 0 to keep the original sample rate. Default: original " +
        "sample rate or 16000 if \"Automatically choose the sample rate\" is checked");
            this.numericUpDownSFX_sample_rate.Value = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            // 
            // numericUpDownSFX_maximum_duration
            // 
            this.numericUpDownSFX_maximum_duration.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownSFX_maximum_duration.Location = new System.Drawing.Point(183, 57);
            this.numericUpDownSFX_maximum_duration.Maximum = new decimal(new int[] {
            30000,
            0,
            0,
            0});
            this.numericUpDownSFX_maximum_duration.Name = "numericUpDownSFX_maximum_duration";
            this.numericUpDownSFX_maximum_duration.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownSFX_maximum_duration.TabIndex = 5;
            this.numericUpDownSFX_maximum_duration.Tag = "max-duration";
            this.toolTip.SetToolTip(this.numericUpDownSFX_maximum_duration, "Maximum sound duration, use 0 to guess automatically. Default: 0");
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(6, 59);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(171, 13);
            this.label21.TabIndex = 4;
            this.label21.Text = "Maximum duration (in milliseconds):";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label20.Location = new System.Drawing.Point(6, 16);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(174, 13);
            this.label20.TabIndex = 3;
            this.label20.Text = "Convert short *.ogg audios to *.wav";
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.groupBox9);
            this.tabPage7.Controls.Add(this.buttonPROFILES_import);
            this.tabPage7.Controls.Add(this.buttonPROFILES_export);
            this.tabPage7.Controls.Add(this.label28);
            this.tabPage7.Controls.Add(this.label27);
            this.tabPage7.Location = new System.Drawing.Point(4, 22);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage7.Size = new System.Drawing.Size(748, 349);
            this.tabPage7.TabIndex = 4;
            this.tabPage7.Text = "Manage conversion profiles";
            this.tabPage7.UseVisualStyleBackColor = true;
            // 
            // groupBox9
            // 
            this.groupBox9.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox9.Controls.Add(this.textBoxPROFILE_filename);
            this.groupBox9.Location = new System.Drawing.Point(9, 86);
            this.groupBox9.Name = "groupBox9";
            this.groupBox9.Size = new System.Drawing.Size(733, 53);
            this.groupBox9.TabIndex = 18;
            this.groupBox9.TabStop = false;
            this.groupBox9.Text = "Loaded profile filename";
            // 
            // textBoxPROFILE_filename
            // 
            this.textBoxPROFILE_filename.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxPROFILE_filename.Location = new System.Drawing.Point(6, 19);
            this.textBoxPROFILE_filename.Name = "textBoxPROFILE_filename";
            this.textBoxPROFILE_filename.ReadOnly = true;
            this.textBoxPROFILE_filename.Size = new System.Drawing.Size(721, 20);
            this.textBoxPROFILE_filename.TabIndex = 0;
            this.textBoxPROFILE_filename.Text = "(no profile loaded)";
            // 
            // buttonPROFILES_import
            // 
            this.buttonPROFILES_import.Location = new System.Drawing.Point(9, 57);
            this.buttonPROFILES_import.Name = "buttonPROFILES_import";
            this.buttonPROFILES_import.Size = new System.Drawing.Size(150, 23);
            this.buttonPROFILES_import.TabIndex = 16;
            this.buttonPROFILES_import.Text = "Import from ini file";
            this.buttonPROFILES_import.UseVisualStyleBackColor = true;
            this.buttonPROFILES_import.Click += new System.EventHandler(this.buttonPROFILES_import_Click);
            // 
            // buttonPROFILES_export
            // 
            this.buttonPROFILES_export.Location = new System.Drawing.Point(165, 57);
            this.buttonPROFILES_export.Name = "buttonPROFILES_export";
            this.buttonPROFILES_export.Size = new System.Drawing.Size(150, 23);
            this.buttonPROFILES_export.TabIndex = 17;
            this.buttonPROFILES_export.Text = "Export current settings";
            this.buttonPROFILES_export.UseVisualStyleBackColor = true;
            this.buttonPROFILES_export.Click += new System.EventHandler(this.buttonPROFILES_export_Click);
            // 
            // label28
            // 
            this.label28.AutoSize = true;
            this.label28.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label28.Location = new System.Drawing.Point(6, 16);
            this.label28.Name = "label28";
            this.label28.Size = new System.Drawing.Size(444, 13);
            this.label28.TabIndex = 15;
            this.label28.Text = "Save and load conversion profiles as *.ini files, this will override or export yo" +
    "ur current settings";
            // 
            // label27
            // 
            this.label27.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label27.AutoSize = true;
            this.label27.Enabled = false;
            this.label27.Font = new System.Drawing.Font("DejaVu Sans", 66F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label27.ForeColor = System.Drawing.SystemColors.ScrollBar;
            this.label27.Location = new System.Drawing.Point(341, 243);
            this.label27.Name = "label27";
            this.label27.Size = new System.Drawing.Size(421, 103);
            this.label27.TabIndex = 14;
            this.label27.Text = "Profiles";
            // 
            // toolTip
            // 
            this.toolTip.AutomaticDelay = 200;
            this.toolTip.AutoPopDelay = 15000;
            this.toolTip.InitialDelay = 200;
            this.toolTip.ReshowDelay = 40;
            this.toolTip.UseFading = false;
            // 
            // label34
            // 
            this.label34.AutoSize = true;
            this.label34.Location = new System.Drawing.Point(6, 125);
            this.label34.Name = "label34";
            this.label34.Size = new System.Drawing.Size(54, 13);
            this.label34.TabIndex = 28;
            this.label34.Text = "GOP size:";
            // 
            // numericUpDownKDM_gop
            // 
            this.numericUpDownKDM_gop.Location = new System.Drawing.Point(103, 123);
            this.numericUpDownKDM_gop.Maximum = new decimal(new int[] {
            256,
            0,
            0,
            0});
            this.numericUpDownKDM_gop.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.numericUpDownKDM_gop.Name = "numericUpDownKDM_gop";
            this.numericUpDownKDM_gop.Size = new System.Drawing.Size(56, 20);
            this.numericUpDownKDM_gop.TabIndex = 29;
            this.numericUpDownKDM_gop.Tag = "gop";
            this.toolTip.SetToolTip(this.numericUpDownKDM_gop, "Changes the GOP size. Default: 15, 18 (with \'--hq\' option) or 12 (with \'--mpeg-bi" +
        "trate\' option)");
            this.numericUpDownKDM_gop.Value = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 522);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.groupBoxFolders);
            this.DoubleBuffered = true;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MainForm";
            this.Text = "Dreamcast Conversion Tool";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.groupBoxFolders.ResumeLayout(false);
            this.groupBoxFolders.PerformLayout();
            this.tabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.contextMenuStrip.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabControlSettings.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.tabPage4.ResumeLayout(false);
            this.tabPage4.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_max_dimmen)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_scale_factor)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_scale_factor_limit)).EndInit();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDT_quality)).EndInit();
            this.tabPage5.ResumeLayout(false);
            this.tabPage5.PerformLayout();
            this.groupBox8.ResumeLayout(false);
            this.groupBox8.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_sample_rate)).EndInit();
            this.groupBox7.ResumeLayout(false);
            this.groupBox7.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_mpeg_bitrate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_two_pass_bitrate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_cue_interval)).EndInit();
            this.tabPage6.ResumeLayout(false);
            this.tabPage6.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSFX_sample_rate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSFX_maximum_duration)).EndInit();
            this.tabPage7.ResumeLayout(false);
            this.tabPage7.PerformLayout();
            this.groupBox9.ResumeLayout(false);
            this.groupBox9.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownKDM_gop)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxFolders;
        private System.Windows.Forms.Button buttonPickInput;
        private System.Windows.Forms.TextBox textBoxFolderOutput;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxFolderInput;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonPickOutput;
        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.ListView listViewFiles;
        private System.Windows.Forms.TabControl tabControlSettings;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.TabPage tabPage6;
        private System.Windows.Forms.TabPage tabPage7;
        private System.Windows.Forms.CheckBox checkBoxBNO_reject_spaces;
        private System.Windows.Forms.TextBox textBoxBNOblacklist;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBoxBNOwhitelist;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox comboBoxKDT_dither_algorithm;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown numericUpDownKDT_quality;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox comboBoxKDT_scale_agorithm;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.CheckBox checkBoxKDT_force_vq_on_small;
        private System.Windows.Forms.CheckBox checkBoxKDT_no_twiddled;
        private System.Windows.Forms.CheckBox checkBoxKDT_vq;
        private System.Windows.Forms.CheckBox checkBoxKDT_rgb565;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.CheckBox checkBoxKDT_force_square_vq;
        private System.Windows.Forms.ComboBox comboBoxKDT_pixel_format;
        private System.Windows.Forms.ComboBox comboBoxKDT_downscale_procedure;
        private System.Windows.Forms.NumericUpDown numericUpDownKDT_scale_factor;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.NumericUpDown numericUpDownKDT_scale_factor_limit;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.ComboBox comboBoxKDM_dither_algorithm;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.ComboBox comboBoxKDM_scale_algorithm;
        private System.Windows.Forms.CheckBox checkBoxKDM_small_resolution;
        private System.Windows.Forms.GroupBox groupBox8;
        private System.Windows.Forms.CheckBox checkBoxKDM_silence;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.NumericUpDown numericUpDownKDM_cue_interval;
        private System.Windows.Forms.CheckBox checkBoxKDM_mono;
        private System.Windows.Forms.NumericUpDown numericUpDownKDM_sample_rate;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.CheckBox checkBoxSFX_auto_sample_rate;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.NumericUpDown numericUpDownSFX_sample_rate;
        private System.Windows.Forms.NumericUpDown numericUpDownSFX_maximum_duration;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.CheckBox checkBoxSFX_pcm_u8;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.CheckBox checkBoxSFX_test_only;
        private System.Windows.Forms.CheckBox checkBoxSFX_copy_if_rejected;
        private System.Windows.Forms.CheckBox checkBoxSFX_force_mono;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.Label label27;
        private System.Windows.Forms.Button buttonPROFILES_import;
        private System.Windows.Forms.Button buttonPROFILES_export;
        private System.Windows.Forms.Label label28;
        private System.Windows.Forms.GroupBox groupBox9;
        private System.Windows.Forms.TextBox textBoxPROFILE_filename;
        private System.Windows.Forms.Button buttonProcess;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.Label label29;
        private System.Windows.Forms.ColumnHeader columnHeaderFiles;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemType;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemOpenFile;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemOpenFolder;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRemove;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox textBoxKDT_image_magick_exec;
        private System.Windows.Forms.Label label30;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRemoveSelected;
        private System.Windows.Forms.CheckBox checkBoxKDM_hq;
        private System.Windows.Forms.CheckBox checkBoxKDM_no_progress;
        private System.Windows.Forms.NumericUpDown numericUpDownKDM_mpeg_bitrate;
        private System.Windows.Forms.Label label31;
        private System.Windows.Forms.NumericUpDown numericUpDownKDM_two_pass_bitrate;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.CheckBox checkBoxKDM_no_save_original_resolution;
        private System.Windows.Forms.ComboBox comboBoxKDM_fps;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label32;
        private System.Windows.Forms.CheckBox checkBoxKDT_uper_slice;
        private System.Windows.Forms.CheckBox checkBoxKDT_sub_slice;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.CheckBox checkBoxKDT_lzss;
        private System.Windows.Forms.CheckBox checkBoxKDT_opacity_slice;
        private System.Windows.Forms.Label label33;
        private System.Windows.Forms.NumericUpDown numericUpDownKDT_max_dimmen;
        private System.Windows.Forms.NumericUpDown numericUpDownKDM_gop;
        private System.Windows.Forms.Label label34;
    }
}

