using Settings;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace ToolConv;

public partial class MainForm : Form
{
    private const string DEFAULT_INI_FILENAME = "last_profile.ini";
    public const int ICON_FILE = 0;
    public const int ICON_BNO = 1;
    public const int ICON_KDT = 2;
    public const int ICON_KDM = 3;
    public const int ICON_SFX = 4;

    private static readonly string[] COMBOBOX_VALUES_DITHER = {
        "auto",
        "bayer",
        "ed",
        "a_dither",
        "x_dither"
    };
    private static readonly string[] COMBOBOX_VALUES_SCALE = {
        "fast_bilinear",
        "bilinear",
        "bicubic",
        "experimental",
        "neighbor",
        "area",
        "bicublin",
        "gauss",
        "sinc",
        "lanczos",
        "spline",
        "print_info",
        "accurate_rnd",
        "full_chroma_int",
        "full_chroma_inp",
        "bitexact",
        "error_diffusion"
    };
    private static readonly string[] COMBOBOX_VALUES_DOWNSCALEPROC = {
        "CLAMP",
        "MEDIUM",
        "HARD"
    };
    private static readonly string[] COMBOBOX_VALUES_PIXELFMT = {
        "AUTO",
        "YUV422",
        "RGB565",
        "ARGB1555",
        "ARGB4444"
    };
    private static readonly string[] COMBOBOX_VALUES_FPS =
    {
        "0",
        "23.976",
        "24.000",
        "25.000",
        "29.970",
        "30.000",
        "50.000",
        "59.940"
    };

    OpenFolderDialog openInputFolderDialog;
    OpenFolderDialog openOutputFolderDialog;

    public MainForm()
    {
        InitializeComponent();

        ImageList imageList = new ImageList();
        imageList.ImageSize = new Size(22, 22);
        imageList.Images.Add(Icons.file);
        imageList.Images.Add(Icons.bno);
        imageList.Images.Add(Icons.kdt);
        imageList.Images.Add(Icons.kdm);
        imageList.Images.Add(Icons.sfx);

        listViewFiles.SmallImageList = imageList;

        columnHeaderFiles.AutoResize(ColumnHeaderAutoResizeStyle.HeaderSize);
        SetWindowTheme(this.listViewFiles.Handle, "Explorer", null);

        openInputFolderDialog = new OpenFolderDialog();
        openInputFolderDialog.Description = "Input folder";
        openInputFolderDialog.ShowNewFolderButton = false;

        openOutputFolderDialog = new OpenFolderDialog();
        openOutputFolderDialog.Description = "Output folder";
    }


    /// <summary>
    /// Punto de entrada principal para la aplicación.
    /// </summary>
    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new MainForm());
    }


    private void LoadProfileFromFile(string filename)
    {
        if (!File.Exists(filename)) return;

        INI ini = new INI(filename);

        foreach (TabPage tab in this.tabControlSettings.TabPages)
        {
            if (tab.Tag is not string) continue;
            string section = (string)tab.Tag;

            LoadProfileToCollection(ini, section, tab.Controls);
        }

        textBoxPROFILE_filename.Text = filename;
    }

    private void LoadProfileToCollection(INI ini, string section, Control.ControlCollection collection)
    {
        foreach (Control control in collection)
        {
            if (control.Tag is not string)
            {
                if (control.HasChildren)
                    LoadProfileToCollection(ini, section, control.Controls);
                continue;
            }

            string key = (string)control.Tag;

            if (!ini.HasKey(section, key)) continue;

            if (control is ComboBox comboBox)
            {
                string[] combobox_values;
                switch (key)
                {
                    case "dither-algorithm":
                        combobox_values = COMBOBOX_VALUES_DITHER;
                        break;
                    case "scale-algorithm":
                        combobox_values = COMBOBOX_VALUES_SCALE;
                        break;
                    case "downscale-procedure":
                        combobox_values = COMBOBOX_VALUES_DOWNSCALEPROC;
                        break;
                    case "pixel-format":
                        combobox_values = COMBOBOX_VALUES_PIXELFMT;
                        break;
                    case "fps":
                        combobox_values = COMBOBOX_VALUES_FPS;
                        break;
                    default:
                        throw new Exception("unknown combox selected item");
                }

                string combobox_value = ini.GetString(section, key, null);
                int idx = Array.IndexOf(combobox_values, combobox_values);

                if (idx >= 0)
                    comboBox.SelectedIndex = idx;
                else
                    comboBox.Text = combobox_value;
            }
            else if (control is TextBox textBox)
            {
                textBox.Text = ini.GetString(section, key, null);
            }
            else if (control is CheckBox checkBox)
            {
                checkBox.Checked = ini.GetBool(section, key, false);
            }
            else if (control is NumericUpDown numericUpDown)
            {
                if (numericUpDown.DecimalPlaces < 1)
                    numericUpDown.Value = (decimal)ini.GetInt(section, key, 0);
                else
                    numericUpDown.Value = (decimal)ini.GetFloat(section, key, Single.NaN);
            }
            else
            {
                // this never should happen
                throw new NotImplementedException("Unknown control type: " + control.GetType().FullName);
            }
        }
    }

    private void SaveProfileToFile(INI ini)
    {
        foreach (TabPage tab in this.tabControlSettings.TabPages)
        {
            if (tab.Tag is not string) continue;
            string section = (string)tab.Tag;

            SaveProfileEntriesFromCollection(ini, section, tab.Controls);
        }

        ini.Flush();
    }

    private void SaveProfileEntriesFromCollection(INI ini, string section, Control.ControlCollection collection)
    {
        foreach (Control control in collection)
        {
            if (control.Tag is not string)
            {
                if (control.HasChildren)
                    SaveProfileEntriesFromCollection(ini, section, control.Controls);
                continue;
            }

            string key = (string)control.Tag;

            if (control is ComboBox comboBox)
            {
                int idx = comboBox.SelectedIndex;

                if (idx >= 0)
                {
                    switch (key)
                    {
                        case "dither-algorithm":
                            ini.SetString(section, key, COMBOBOX_VALUES_DITHER[idx]);
                            break;
                        case "scale-algorithm":
                            ini.SetString(section, key, COMBOBOX_VALUES_SCALE[idx]);
                            break;
                        case "downscale-procedure":
                            ini.SetString(section, key, COMBOBOX_VALUES_DOWNSCALEPROC[idx]);
                            break;
                        case "pixel-format":
                            ini.SetString(section, key, COMBOBOX_VALUES_PIXELFMT[idx]);
                            break;
                        case "fps":
                            ini.SetString(section, key, COMBOBOX_VALUES_FPS[idx]);
                            break;
                        default:
                            throw new Exception("unknown combox selected item");
                    }
                }
                else
                {
                    ini.SetString(section, key, null);
                }
            }
            else if (control is TextBox textBox)
            {
                ini.SetString(section, key, textBox.Text);
            }
            else if (control is CheckBox checkBox)
            {
                ini.SetBool(section, key, checkBox.Checked);
            }
            else if (control is NumericUpDown numericUpDown)
            {
                if (numericUpDown.DecimalPlaces < 1)
                    ini.SetInt(section, key, (int)numericUpDown.Value);
                else
                    ini.SetFloat(section, key, (float)numericUpDown.Value);
            }
            else
            {
                // this never should happen
                throw new NotImplementedException("Unknown control type: " + control.GetType().FullName);
            }
        }

    }

    private void AddFile(string file, string parent_folder)
    {
        string relative_path;
        if (parent_folder != null)
        {
            relative_path = file.Substring(parent_folder.Length);
            if (relative_path.Length > 0 && relative_path[0] == Path.DirectorySeparatorChar)
            {
                relative_path = relative_path.Substring(1);
            }
        }
        else
        {
            relative_path = file;
        }

        ListViewItem item = new ListViewItem();
        item.Tag = file;
        item.Text = "  " + relative_path;
        item.ImageIndex = ICON_FILE;

        string ext = Path.GetExtension(file.ToLowerInvariant());

        switch (ext)
        {
            case ".json":
            case ".xml":
                item.ImageIndex = ICON_BNO;
                break;
            case ".jpeg":
            case ".jpg":
            case ".png":
            case ".ico":
            case ".bmp":
            case ".webp":
            case ".dds":
                item.ImageIndex = ICON_KDT;
                break;
            case ".mp4":
            case ".m4v":
            case ".3gp":
            case ".webm":
            case ".mkv":
                item.ImageIndex = ICON_KDM;
                break;
            case ".ogg":
            case ".logg":
                item.ImageIndex = ICON_SFX;
                break;
        }

        listViewFiles.Items.Add(item);
    }

    private void AddFolder(string folder)
    {
        Queue<DirectoryInfo> folders = new Queue<DirectoryInfo>(128);
        List<string> files = new List<string>(256);

        folders.Enqueue(new DirectoryInfo(folder));

        while (folders.Count > 0)
        {
            DirectoryInfo folder_info = folders.Dequeue();

            foreach (DirectoryInfo subfolder in folder_info.EnumerateDirectories())
            {
                folders.Enqueue(subfolder);
            }

            foreach (FileInfo file in folder_info.EnumerateFiles())
            {
                files.Add(file.FullName);
            }

        }

        //files.Sort(new AlphanumComparator.AlphanumComparator());

        foreach (string file in files)
        {
            AddFile(file, folder);
        }
    }

    private void SetOutputFolder(string folder)
    {
        IEnumerable<string> items = Directory.EnumerateFileSystemEntries(folder);
        using (IEnumerator<string> en = items.GetEnumerator())
        {
            if (en.MoveNext())
            {
                DialogResult ret = MessageBox.Show(
                      "The selected folder is not empty, some files may be overwritten.\r\n¿Continue?",
                      "Warning",
                      MessageBoxButtons.YesNo,
                      MessageBoxIcon.Warning
                  );

                if (ret == DialogResult.No) return;
            }
        }
        textBoxFolderOutput.Text = folder;
    }

    private void SetInputFolder(string folder)
    {
        textBoxFolderInput.Text = folder;

        try
        {
            groupBoxFolders.Enabled = tabControl.Enabled = false;
            listViewFiles.Items.Clear();
            AddFolder(folder);
        }
        finally
        {
            groupBoxFolders.Enabled = tabControl.Enabled = true;
        }
    }

    private static void OpenAndSelect(string path)
    {
        string foldername, filename;

        if (Directory.Exists(path))
        {
            foldername = path;
            filename = null;
        }
        else if (File.Exists(path))
        {
            FileInfo file_info = new FileInfo(path);
            foldername = file_info.DirectoryName;
            filename = file_info.Name;
        }
        else
        {
            MessageBox.Show("Open failed", "Path not longer valid:\r\n" + path, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            return;
        }

        nint folder;
        nint[] files = new nint[1];
        uint file_count = 0;

        SHParseDisplayName(foldername, 0x00, out folder, 0, out _);

        if (filename != null)
        {
            SHParseDisplayName(path, 0x00, out files[0], 0, out _);
            if (files[0] != 0x00) file_count++;
        }

        if (folder != 0x00)
        {
            SHOpenFolderAndSelectItems(folder, file_count, files, 0);
        }

        if (folder != 0x00) Marshal.FreeCoTaskMem(folder);
        if (files[0] != 0x00) Marshal.FreeCoTaskMem(files[0]);
    }



    private void buttonPROFILES_import_Click(object sender, EventArgs e)
    {
        using (OpenFileDialog dialog = new OpenFileDialog())
        {
            dialog.Filter = "INI|*.ini";
            dialog.Title = "Open Profile";
            DialogResult ret = dialog.ShowDialog();

            if (ret == DialogResult.OK)
            {
                LoadProfileFromFile(dialog.FileName);
                textBoxPROFILE_filename.Text = dialog.FileName;
            }
        }
    }

    private void buttonPROFILES_export_Click(object sender, EventArgs e)
    {
        using (SaveFileDialog dialog = new SaveFileDialog())
        {
            dialog.Filter = "INI|*.ini";
            dialog.Title = "Save Profile";
            DialogResult ret = dialog.ShowDialog();

            if (ret == DialogResult.OK)
            {
                SaveProfileToFile(new INI(dialog.FileName));
                textBoxPROFILE_filename.Text = dialog.FileName;
            }
        }
    }

    private void MainForm_Load(object sender, EventArgs e)
    {
        LoadProfileFromFile(AppDomain.CurrentDomain.BaseDirectory + DEFAULT_INI_FILENAME);
    }

    private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
    {
        INI ini = new INI(AppDomain.CurrentDomain.BaseDirectory + DEFAULT_INI_FILENAME);
        SaveProfileToFile(ini);
    }

    private void buttonPickInput_Click(object sender, EventArgs e)
    {
        openInputFolderDialog.Description = "Input folder";
        openInputFolderDialog.ShowNewFolderButton = false;
        DialogResult ret = openInputFolderDialog.ShowDialog();

        if (ret == DialogResult.OK)
        {
            SetInputFolder(openInputFolderDialog.SelectedPath);
        }
    }

    private void buttonPickOutput_Click(object sender, EventArgs e)
    {
        DialogResult ret = openInputFolderDialog.ShowDialog();
        openOutputFolderDialog.Description = "Output folder";

        if (ret == DialogResult.OK)
        {
            SetOutputFolder(openInputFolderDialog.SelectedPath);
        }
    }

    private void textBoxFolder_DragEnter(object sender, DragEventArgs e)
    {
        if (e.Data.GetDataPresent(DataFormats.FileDrop))
        {
            string[] paths = (string[])e.Data.GetData(DataFormats.FileDrop);

            if (paths.Length == 1 && Directory.Exists(paths[0]))
            {
                e.Effect = DragDropEffects.Link;
                return;
            }
            else if (paths.Length > 1 && sender == textBoxFolderInput)
            {
                int count = 0;

                foreach (string path in paths)
                    if (Directory.Exists(path))
                        count++;

                if (paths.Length == count)
                {
                    e.Effect = DragDropEffects.Link;
                    return;
                }
            }
        }

        e.Effect = DragDropEffects.None;
    }

    private void textBoxFolder_DragDrop(object sender, DragEventArgs e)
    {
        string[] paths = (string[])e.Data.GetData(DataFormats.FileDrop);

        if (paths.Length == 1)
        {
            if (Directory.Exists(paths[0]))
            {
                if (sender == textBoxFolderInput)
                    SetInputFolder(paths[0]);
                else if (sender == textBoxFolderOutput)
                    SetOutputFolder(paths[0]);
                else
                    throw new InvalidOperationException("Unknown drag-n-drop target");
            }
        }
        else if (sender == textBoxFolderInput)
        {
            tabPage1_DragDrop(sender, e);
            textBoxFolderInput.Text = "";
        }
    }

    private void tabPage1_DragEnter(object sender, DragEventArgs e)
    {
        if (e.Data.GetDataPresent(DataFormats.FileDrop))
        {
            string[] paths = (string[])e.Data.GetData(DataFormats.FileDrop);
            int count = 0;

            foreach (string path in paths)
            {
                if (File.Exists(path) || Directory.Exists(path))
                {
                    count++;
                }
            }

            if (count == paths.Length)
            {
                e.Effect = DragDropEffects.Link;
                return;
            }
        }

        e.Effect = DragDropEffects.None;
    }

    private void tabPage1_DragDrop(object sender, DragEventArgs e)
    {
        string[] paths = (string[])e.Data.GetData(DataFormats.FileDrop);

        foreach (string path in paths)
        {
            groupBoxFolders.Enabled = tabControl.Enabled = false;

            try
            {
                if (File.Exists(path))
                {
                    AddFile(path, null);
                }
                else if (Directory.Exists(path))
                {
                    AddFolder(path);
                }
            }
            finally
            {
                groupBoxFolders.Enabled = tabControl.Enabled = true;
            }
        }

    }

    private void listViewFiles_KeyUp(object sender, KeyEventArgs e)
    {
        if (e.KeyCode == Keys.Delete && e.Modifiers == Keys.None)
        {
            toolStripMenuItemMenuRemoveSelected_Click(sender, e);
            return;
        }
        else if ((e.KeyCode == Keys.A && e.Modifiers == Keys.Control) || (e.KeyCode == Keys.E && e.Modifiers == Keys.Control))
        {
            foreach (ListViewItem item in listViewFiles.Items) item.Selected = true;
        }
        else if ((e.KeyCode == Keys.I && e.Modifiers == Keys.Control) || (e.KeyCode == Keys.A && e.Modifiers == (Keys.Control | Keys.Shift)))
        {
            foreach (ListViewItem item in listViewFiles.Items) item.Selected = !item.Selected;
        }
    }

    private void buttonProcess_Click(object sender, EventArgs e)
    {
        if (textBoxFolderInput.TextLength < 1 && listViewFiles.Items.Count < 1)
        {
            MessageBox.Show(buttonProcess.Text, "Select an input folder", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
            return;
        }
        if (textBoxFolderOutput.TextLength < 1)
        {
            MessageBox.Show(buttonProcess.Text, "Select an output folder", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
            return;
        }
        if (listViewFiles.Items.Count < 1)
        {
            MessageBox.Show(buttonProcess.Text, "There no files to process", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
            return;
        }


        INI settings = new INI(null);
        SaveProfileToFile(settings);

        this.Hide();
        using (ProcessForm processForm = new ProcessForm())
        {
            processForm.SetItemsToProcess(listViewFiles.Items, settings, textBoxFolderInput.Text, textBoxFolderOutput.Text, this);
            processForm.ShowDialog(this);
        }
        this.Show();
    }

    private void toolStripMenuItemOpenFile_Click(object sender, EventArgs e)
    {
        string path = (string)listViewFiles.SelectedItems[0].Tag;
        Process.Start(new ProcessStartInfo() { UseShellExecute = true, FileName = path });
    }

    private void toolStripMenuItemOpenFolder_Click(object sender, EventArgs e)
    {
        string path = (string)listViewFiles.SelectedItems[0].Tag;
        OpenAndSelect(path);
    }

    private void toolStripMenuItemRemove_Click(object sender, EventArgs e)
    {
        listViewFiles.SelectedItems[0].Remove();
    }

    private void toolStripMenuItemMenuRemoveSelected_Click(object sender, EventArgs e)
    {
        int[] indices = new int[listViewFiles.SelectedIndices.Count];
        for (int i = 0; i < indices.Length; i++)
        {
            indices[i] = listViewFiles.SelectedIndices[i];
        }

        for (int i = indices.Length - 1; i >= 0; i--)
        {
            listViewFiles.Items.RemoveAt(indices[i]);
        }
    }

    private void listViewFiles_DoubleClick(object sender, EventArgs e)
    {
        if (listViewFiles.SelectedItems.Count == 1)
        {
            toolStripMenuItemOpenFile_Click(sender, e);
        }
    }

    private void contextMenuStrip_Opening(object sender, CancelEventArgs e)
    {
        if (listViewFiles.SelectedItems.Count < 1)
        {
            e.Cancel = true;
            return;
        }


        if (listViewFiles.SelectedItems.Count == 1)
        {
            switch (listViewFiles.SelectedItems[0].ImageIndex)
            {
                case ICON_FILE:
                    toolStripMenuItemType.Text = "  File (it will be copied)";
                    break;
                case ICON_BNO:
                    toolStripMenuItemType.Text = "  JSON/XML";
                    break;
                case ICON_KDT:
                    toolStripMenuItemType.Text = "  Image/Texture";
                    break;
                case ICON_KDM:
                    toolStripMenuItemType.Text = "  Video/Media";
                    break;
                case ICON_SFX:
                    toolStripMenuItemType.Text = "  Sound";
                    break;
            }
        }

        toolStripMenuItemOpenFile.Visible = listViewFiles.SelectedItems.Count == 1;
        toolStripMenuItemOpenFolder.Visible = listViewFiles.SelectedItems.Count == 1;
        toolStripMenuItemType.Visible = listViewFiles.SelectedItems.Count == 1;
        toolStripSeparator.Visible = listViewFiles.SelectedItems.Count == 1;
        toolStripMenuItemRemove.Visible = listViewFiles.SelectedItems.Count == 1;
        toolStripMenuItemRemoveSelected.Visible = listViewFiles.SelectedItems.Count > 0;
    }


    [DllImport("uxtheme", ExactSpelling = true, CharSet = CharSet.Unicode)]
    private static extern int SetWindowTheme(nint hwnd, string pszSubAppName, string pszSubIdList);

    [DllImport("shell32", SetLastError = true)]
    private static extern int SHOpenFolderAndSelectItems(nint pidlFolder, uint cidl, [In, MarshalAs(UnmanagedType.LPArray)] nint[] apidl, uint dwFlags);

    [DllImport("shell32", SetLastError = true)]
    private static extern void SHParseDisplayName([MarshalAs(UnmanagedType.LPWStr)] string name, nint bindingContext, [Out] out nint pidl, uint sfgaoIn, [Out] out uint psfgaoOut);

}
