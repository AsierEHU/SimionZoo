﻿using System.IO;
using System.Xml;

using Herd.Files;

namespace Badger.ViewModels
{
    public class BranchConfigViewModel : NestedConfigNode
    {
        private string m_className = "";

        public string ClassName
        {
            get { return m_className; }
            set
            {
                m_className = value;
                NotifyOfPropertyChange(() => ClassName);
            }
        }

        private string m_window = "";

        private bool m_bIsOptional = false;
        public bool bIsOptional
        {
            get { return m_bIsOptional; }
            set { m_bIsOptional = value; NotifyOfPropertyChange(() => bIsOptional); }
        }

        private bool m_bIsUsed = false;
        public bool bIsUsed
        {
            get { return !bIsOptional || m_bIsUsed; }
            set { m_bIsUsed = value; NotifyOfPropertyChange(() => bIsUsed); }
        }

        public BranchConfigViewModel(ExperimentViewModel parentExperiment, ConfigNodeViewModel parent
            , XmlNode definitionNode, string parentXPath, XmlNode configNode = null, bool initChildren = true)
        {
            CommonInitialization(parentExperiment, parent, definitionNode, parentXPath);

            m_className = definitionNode.Attributes[XMLTags.classAttribute].Value;
            if (definitionNode.Attributes.GetNamedItem(XMLTags.windowAttribute) != null)
                m_window = definitionNode.Attributes[XMLTags.windowAttribute].Value;
            else m_window = "";
            if (definitionNode.Attributes.GetNamedItem(XMLTags.optionalAttribute) != null)
                bIsOptional = bool.Parse(definitionNode.Attributes[XMLTags.optionalAttribute].Value);
            else bIsOptional = false;

            if (configNode != null)
            {
                configNode = configNode[name];
                if (configNode != null && bIsOptional)
                    bIsUsed = true;
            }
            else if (bIsOptional)
                bIsUsed = true;

            if (initChildren)
                childrenInit(parentExperiment, parentExperiment.GetClassDefinition(m_className), m_xPath, configNode);
        }

        public override bool Validate()
        {
            if (bIsOptional && !bIsUsed) return true; // don't need to validate children if its optional and not used

            foreach (ConfigNodeViewModel child in children)
                if (!child.Validate())
                {
                    //Console.WriteLine(child.name);
                    return false;
                }
            return true;
        }

        public override void outputXML(StreamWriter writer, SaveMode mode, string leftSpace)
        {
            if (bIsUsed)
                base.outputXML(writer, mode, leftSpace);
        }
        public override ConfigNodeViewModel clone()
        {
            BranchConfigViewModel newBranch = new BranchConfigViewModel(m_parentExperiment, parent,
                nodeDefinition, parent.xPath, null, false);
            foreach (ConfigNodeViewModel child in children)
            {
                ConfigNodeViewModel clonedChild = child.clone();
                clonedChild.parent = newBranch;
                newBranch.children.Add(clonedChild);
            }
            return newBranch;
        }
    }
}
