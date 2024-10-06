namespace ToolConv
{
    partial class ProcessForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ProcessForm));
            this.listBoxProcessed = new System.Windows.Forms.ListBox();
            this.textBoxStdOut = new System.Windows.Forms.TextBox();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.labelCompleted = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.labelStatus = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.labelFile = new System.Windows.Forms.Label();
            this.progressBarLongTask = new System.Windows.Forms.ProgressBar();
            this.labelLongTaskEstimated = new System.Windows.Forms.Label();
            this.labelLongTaskHint = new System.Windows.Forms.Label();
            this.backgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // listBoxProcessed
            // 
            this.listBoxProcessed.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listBoxProcessed.DisplayMember = "Text";
            this.listBoxProcessed.FormattingEnabled = true;
            this.listBoxProcessed.Location = new System.Drawing.Point(12, 12);
            this.listBoxProcessed.Name = "listBoxProcessed";
            this.listBoxProcessed.Size = new System.Drawing.Size(776, 225);
            this.listBoxProcessed.TabIndex = 0;
            this.listBoxProcessed.SelectedIndexChanged += new System.EventHandler(this.listBoxProcessed_SelectedIndexChanged);
            // 
            // textBoxStdOut
            // 
            this.textBoxStdOut.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStdOut.Location = new System.Drawing.Point(12, 243);
            this.textBoxStdOut.Multiline = true;
            this.textBoxStdOut.Name = "textBoxStdOut";
            this.textBoxStdOut.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxStdOut.Size = new System.Drawing.Size(776, 90);
            this.textBoxStdOut.TabIndex = 1;
            // 
            // progressBar
            // 
            this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBar.Location = new System.Drawing.Point(12, 437);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(776, 23);
            this.progressBar.TabIndex = 2;
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.Location = new System.Drawing.Point(638, 476);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(150, 23);
            this.buttonCancel.TabIndex = 3;
            this.buttonCancel.Tag = "Close";
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 421);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Completed:";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label2.Location = new System.Drawing.Point(12, 406);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(776, 2);
            this.label2.TabIndex = 5;
            // 
            // labelCompleted
            // 
            this.labelCompleted.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.labelCompleted.Location = new System.Drawing.Point(713, 421);
            this.labelCompleted.Name = "labelCompleted";
            this.labelCompleted.Size = new System.Drawing.Size(73, 13);
            this.labelCompleted.TabIndex = 6;
            this.labelCompleted.Text = "###/###";
            this.labelCompleted.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 374);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(40, 13);
            this.label5.TabIndex = 7;
            this.label5.Text = "Status:";
            // 
            // labelStatus
            // 
            this.labelStatus.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelStatus.AutoSize = true;
            this.labelStatus.Location = new System.Drawing.Point(134, 377);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(147, 13);
            this.labelStatus.TabIndex = 9;
            this.labelStatus.Text = "####################";
            // 
            // label6
            // 
            this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 348);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(26, 13);
            this.label6.TabIndex = 10;
            this.label6.Text = "File:";
            // 
            // labelFile
            // 
            this.labelFile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelFile.AutoSize = true;
            this.labelFile.Location = new System.Drawing.Point(134, 348);
            this.labelFile.Name = "labelFile";
            this.labelFile.Size = new System.Drawing.Size(126, 13);
            this.labelFile.TabIndex = 9;
            this.labelFile.Text = "#################";
            // 
            // progressBarLongTask
            // 
            this.progressBarLongTask.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBarLongTask.Location = new System.Drawing.Point(288, 374);
            this.progressBarLongTask.Maximum = 10000;
            this.progressBarLongTask.Name = "progressBarLongTask";
            this.progressBarLongTask.Size = new System.Drawing.Size(498, 16);
            this.progressBarLongTask.Step = 1;
            this.progressBarLongTask.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.progressBarLongTask.TabIndex = 11;
            this.progressBarLongTask.Visible = false;
            // 
            // labelLongTaskEstimated
            // 
            this.labelLongTaskEstimated.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.labelLongTaskEstimated.Location = new System.Drawing.Point(710, 358);
            this.labelLongTaskEstimated.Name = "labelLongTaskEstimated";
            this.labelLongTaskEstimated.Size = new System.Drawing.Size(76, 13);
            this.labelLongTaskEstimated.TabIndex = 12;
            this.labelLongTaskEstimated.Text = "##:##";
            this.labelLongTaskEstimated.TextAlign = System.Drawing.ContentAlignment.TopRight;
            this.labelLongTaskEstimated.Visible = false;
            // 
            // labelLongTaskHint
            // 
            this.labelLongTaskHint.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.labelLongTaskHint.AutoSize = true;
            this.labelLongTaskHint.Location = new System.Drawing.Point(578, 358);
            this.labelLongTaskHint.Name = "labelLongTaskHint";
            this.labelLongTaskHint.Size = new System.Drawing.Size(126, 13);
            this.labelLongTaskHint.TabIndex = 13;
            this.labelLongTaskHint.Text = "Estimated remaining time:";
            this.labelLongTaskHint.Visible = false;
            // 
            // backgroundWorker
            // 
            this.backgroundWorker.WorkerReportsProgress = true;
            this.backgroundWorker.WorkerSupportsCancellation = true;
            this.backgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker_DoWork);
            this.backgroundWorker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorker_ProgressChanged);
            this.backgroundWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker_RunWorkerCompleted);
            // 
            // timer
            // 
            this.timer.Enabled = true;
            this.timer.Interval = 250;
            this.timer.Tick += new System.EventHandler(this.timer_Tick);
            // 
            // ProcessForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 511);
            this.Controls.Add(this.labelLongTaskHint);
            this.Controls.Add(this.labelLongTaskEstimated);
            this.Controls.Add(this.progressBarLongTask);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.labelFile);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.labelCompleted);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.textBoxStdOut);
            this.Controls.Add(this.listBoxProcessed);
            this.DoubleBuffered = true;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(816, 550);
            this.Name = "ProcessForm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Tag = "Completed processing files";
            this.Text = "Dreamcast Conversion Tool - Processing files...";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ProcessForm_FormClosing);
            this.Shown += new System.EventHandler(this.ProcessForm_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxProcessed;
        private System.Windows.Forms.TextBox textBoxStdOut;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labelCompleted;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label labelFile;
        private System.Windows.Forms.ProgressBar progressBarLongTask;
        private System.Windows.Forms.Label labelLongTaskEstimated;
        private System.Windows.Forms.Label labelLongTaskHint;
        private System.ComponentModel.BackgroundWorker backgroundWorker;
        private System.Windows.Forms.Timer timer;
    }
}