using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Windows.Forms;


internal class OpenFolderDialog : IDisposable
{
    private const uint FOS_PICKFOLDERS = 0x00000020;
    private const uint FOS_FORCEFILESYSTEM = 0x00000040;
    private const uint FOS_NOVALIDATE = 0x00000100;
    private const uint FOS_NOTESTFILECREATE = 0x00010000;
    private const uint FOS_DONTADDTORECENT = 0x02000000;
    private const uint S_OK = 0x0000;
    private const uint SIGDN_FILESYSPATH = 0x80058000;

    private Guid IShellItem_GUID = new Guid("43826D1E-E718-42EE-BC55-A1E261C37BFE");


    private readonly FolderBrowserDialog legacy;


    public OpenFolderDialog()
    {
        this.legacy = new FolderBrowserDialog();
    }


    public string Description
    {
        get => this.legacy.Description; set => this.legacy.Description = value;
    }

    public Environment.SpecialFolder RootFolder
    {
        get => this.legacy.RootFolder; set => this.legacy.RootFolder = value;
    }

    public string SelectedPath
    {
        get => this.legacy.SelectedPath; set => this.legacy.SelectedPath = value;
    }

    public bool ShowNewFolderButton
    {
        get => this.legacy.ShowNewFolderButton; set => this.legacy.ShowNewFolderButton = value;
    }

    public ISite Site
    {
        get => this.legacy.Site; set => this.legacy.Site = value;
    }

    public object Tag
    {
        get => this.legacy.Tag; set => this.legacy.Tag = value;
    }


    public void Reset()
    {
        this.legacy.Reset();
    }

    public void Dispose()
    {
        this.legacy.Dispose();
    }

    internal DialogResult ShowDialog()
    {
        return ShowDialog(null);
    }

    internal DialogResult ShowDialog(IWin32Window owner)
    {
        if (Environment.OSVersion.Version.Major < 6)
        {
            return this.legacy.ShowDialog(owner);
        }

        nint owner_handle = 0x00;
        NativeWindow nativeWindow = null;

        if (owner != null)
        {
            if (owner is Control)
                owner_handle = ((Control)(owner)).Handle;
            else
                owner_handle = owner.Handle;
        }

        if (owner_handle == 0x00)
        {
            owner_handle = GetActiveWindow();
        }

        if (owner_handle == 0x00)
        {
            nativeWindow = new NativeWindow();
            nativeWindow.CreateHandle(new CreateParams());
            owner_handle = nativeWindow.Handle;
        }

        string initialFolder = this.legacy.SelectedPath;
        if (String.IsNullOrEmpty(initialFolder)) initialFolder = Environment.GetFolderPath(this.legacy.RootFolder);

        IFileDialog dialog = (IFileDialog)new FileOpenDialogRCW();

        uint options;
        dialog.GetOptions(out options);
        dialog.SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOTESTFILECREATE | FOS_DONTADDTORECENT);

        IShellItem shellItem;
        if (SHCreateItemFromParsingName(initialFolder, 0x00, ref IShellItem_GUID, out shellItem) == S_OK)
        {
            dialog.SetDefaultFolder(shellItem);
        }

        if (!String.IsNullOrEmpty(this.legacy.Description))
        {
            dialog.SetTitle(this.Description);
        }

        DialogResult result = DialogResult.Cancel;
        nint string_ptr = 0x00;

        if (dialog.Show(owner_handle) != S_OK)
        {
            goto L_return;
        }

        if (dialog.GetResult(out shellItem) != S_OK)
        {
            goto L_return;
        }

        if (shellItem.GetDisplayName(SIGDN_FILESYSPATH, out string_ptr) != S_OK)
        {
            goto L_return;
        }

        this.SelectedPath = Marshal.PtrToStringAuto(string_ptr);
        result = DialogResult.OK;

    L_return:
        if (nativeWindow != null) nativeWindow.DestroyHandle();
        if (string_ptr != 0x00) Marshal.FreeCoTaskMem(string_ptr);
        return result;
    }


    [ComImport, Guid("42F85136-DB7E-439C-85F1-E4075D135FC8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IFileDialog
    {
        [PreserveSig] uint Show(nint hwndOwner);

        uint SetFileTypes(uint cFileTypes, nint rgFilterSpec);

        uint SetFileTypeIndex(uint iFileType);

        uint GetFileTypeIndex(out uint piFileType);

        uint Advise([MarshalAs(UnmanagedType.Interface)] nint pfde, out uint pdwCookie);

        uint Unadvise(uint dwCookie);

        uint SetOptions(uint fos);

        uint GetOptions(out uint fos);

        void SetDefaultFolder([MarshalAs(UnmanagedType.Interface)] IShellItem psi);

        uint SetFolder([MarshalAs(UnmanagedType.Interface)] IShellItem psi);

        uint GetFolder([MarshalAs(UnmanagedType.Interface)] out IShellItem ppsi);

        uint GetCurrentSelection([MarshalAs(UnmanagedType.Interface)] out IShellItem ppsi);

        uint SetFileName([MarshalAs(UnmanagedType.LPWStr)] string pszName);

        uint GetFileName([MarshalAs(UnmanagedType.LPWStr)] out string pszName);

        uint SetTitle([MarshalAs(UnmanagedType.LPWStr)] string pszTitle);

        uint SetOkButtonLabel([MarshalAs(UnmanagedType.LPWStr)] string pszText);

        uint SetFileNameLabel([MarshalAs(UnmanagedType.LPWStr)] string pszLabel);

        uint GetResult(out IShellItem ppsi);

        uint AddPlace(IShellItem psi, uint fdap);

        uint SetDefaultExtension([MarshalAs(UnmanagedType.LPWStr)] string pszDefaultExtension);

        uint Close([MarshalAs(UnmanagedType.Error)] uint hr);

        uint SetClientGuid([In] ref Guid guid);

        uint ClearClientData();

        uint SetFilter(nint pFilter);
    }

    [ComImport, ClassInterface(ClassInterfaceType.None), TypeLibType(TypeLibTypeFlags.FCanCreate), Guid("DC1C5A9C-E88A-4DDE-A5A1-60F82A20AEF7")]
    internal class FileOpenDialogRCW { }

    [ComImport, Guid("43826D1E-E718-42EE-BC55-A1E261C37BFE"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IShellItem
    {
        uint BindToHandler(nint pbc, ref Guid rbhid, ref Guid riid, out nint ppvOut);
        uint GetParent(out IShellItem ppsi);
        uint GetDisplayName(uint sigdnName, out nint ppszName);
        uint GetAttributes(uint sfgaoMask, out uint psfgaoAttribs);
        uint Compare(IShellItem psi, uint hint, out int piOrder);
    }


    [DllImport("user32", SetLastError = true)]
    private static extern nint GetActiveWindow();

    [DllImport("shell32", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern int SHCreateItemFromParsingName([MarshalAs(UnmanagedType.LPWStr)] string pszPath, nint pbc, ref Guid riid, [MarshalAs(UnmanagedType.Interface)] out IShellItem ppv);

}
