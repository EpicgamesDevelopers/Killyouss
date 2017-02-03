﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UnrealBuildTool
{
	/// <summary>
	/// Public Mac functions exposed to UAT
	/// </summary>
	public class MacExports
	{
		/// <summary>
		/// Strips symbols from a file
		/// </summary>
		/// <param name="SourceFileName">The input file</param>
		/// <param name="TargetFileName">The output file</param>
		public static void StripSymbols(string SourceFileName, string TargetFileName)
		{
			MacToolChain ToolChain = new MacToolChain(null);
			ToolChain.StripSymbols(SourceFileName, TargetFileName);
		}
	}
}