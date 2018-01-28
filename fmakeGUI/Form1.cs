using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace fmakeGUI
{
    public partial class Form1 : Form
    {
        string targetDirectory;
        string includeDirectory;
        string sourceDirectory;

        string selectDirectory;
        string SelectDirectory { get { return selectDirectory; } set
            {
                selectDirectory = value;
                UpdateNamespace();
            } }

        public Form1()
        {
            targetDirectory = Directory.GetCurrentDirectory() + @"\";
            includeDirectory = targetDirectory + @"include\";
            sourceDirectory = targetDirectory + @"src\";

            InitializeComponent();

            treeView1.NodeMouseDoubleClick += TreeView1_NodeMouseDoubleClick;

            this.ActiveControl = button3;
        }

        private void TreeView1_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            var name = e.Node.FullPath;
            if(checkBox2.Checked)
            {
                Process.Start(includeDirectory + name + @"\");
            }
            if(checkBox3.Checked)
            {
                Process.Start(sourceDirectory + name + @"\");
            }
        }

        public bool Init()
        {
            var directoryInfo = new DirectoryInfo(targetDirectory);
            var subFolders = directoryInfo.EnumerateDirectories("*", SearchOption.TopDirectoryOnly);
            int success = 0;
            foreach (DirectoryInfo subFolder in subFolders)
            {
                var fullname = subFolder.FullName;
                var name = fullname.Substring(targetDirectory.Length);
                switch (name)
                {
                    case "assets":
                    case "include":
                    case "resources":
                    case "src":
                    case "vc2017":
                        success += 1;
                        break;
                    default:
                        break;
                }
            }
            return success >= 5;
        }

        public void Setup()
        {
            SelectDirectory = includeDirectory;

            treeView1.HideSelection = false;

            var directoryInfo = new DirectoryInfo(includeDirectory);
            var subFolders = directoryInfo.EnumerateDirectories("*", SearchOption.AllDirectories);
            foreach (DirectoryInfo subFolder in subFolders)
            {
                var fullname = subFolder.FullName;
                var name = fullname.Substring(includeDirectory.Length);
                var children = treeView1.Nodes;
                foreach(var n in name.Split('\\'))
                {
                    if (children.ContainsKey(n))
                    {
                        children = children[n].Nodes;
                    }
                    else
                    {
                        children = children.Add(n, n).Nodes;
                    }
                }
            }

            UpdateNamespace();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var f = new Form2(SelectDirectory.Substring(includeDirectory.Length));
            if (f.ShowDialog() == DialogResult.OK)
            {
                var fullname = SelectDirectory + f.ResultText;
                var name = fullname.Substring(includeDirectory.Length);
                var children = treeView1.Nodes;
                foreach (var n in name.Split('\\'))
                {
                    if (children.ContainsKey(n))
                    {
                        children = children[n].Nodes;
                    }
                    else
                    {
                        var node = children.Add(n, n);
                        if(node.Parent != null)
                        {
                            node.Parent.Expand();
                        }
                        treeView1.SelectedNode = node;
                        treeView1.Focus();
                        children = node.Nodes;
                    }
                }
            }
        }

        private void UpdateNamespace()
        {
            var resultNamespace = SelectDirectory.Substring(includeDirectory.Length);
            var ns = resultNamespace.Split('\\');
            string result = "::";
            foreach (var n in ns)
            {
                if(n.Length != 0)
                {
                    result += n + "::";
                }
            }
            result += textBox1.Text;
            label4.Text = result;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            string program = targetDirectory + @"fmake.exe";

            Process extProcess = new Process();
            extProcess.StartInfo.FileName = program;    //起動するファイル名
            extProcess.StartInfo.WorkingDirectory = targetDirectory;

            string argNamespace = SelectDirectory.Substring(includeDirectory.Length);
            string argFilename = textBox1.Text;
            string argMode = checkBox1.Checked ? "h" : "hcpp";

            extProcess.StartInfo.Arguments = argNamespace + " " + argFilename + " " + argMode;

            extProcess.Start();
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var name = e.Node.FullPath;
            SelectDirectory = includeDirectory + name + @"\";
        }

        private void button3_Click(object sender, EventArgs e)
        {
            treeView1.SelectedNode = null;
            SelectDirectory = includeDirectory;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            UpdateNamespace();
        }
    }
}
